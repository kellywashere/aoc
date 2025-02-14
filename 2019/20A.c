#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

enum content {
	EMPTY,
	FLOOR,
	WALL
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct cell {
	enum content content;
	int          label;  // 0: no label
	int          portalidx; // portal connection
};

struct grid {
	int          gridw;
	int          gridh;
	struct cell* grid;

	int          startidx;
	int          targetidx;
};

#define MAX_LINES 256
#define MAX_LINE_LEN 256

#define LABEL_AA 1000
#define LABEL_ZZ (1000 + 26*26-1)

/************************* MINHEAP *******************************/
// NOTE: this implementation allows searching for value, and
// changing key of a value to allow A-star search !
struct minheap_el {
	int key;
	int val;
};

struct minheap {
	struct minheap_el* mem;      // array-based min heap
	unsigned int       size;     // nr elements currently in heap
	unsigned int       capacity; // capacity in nr of elements
};

struct minheap* create_minheap() {
	struct minheap* heap = malloc(sizeof(struct minheap));
	heap->capacity = 127; // 7 layers start size
	heap->mem = malloc(sizeof(struct minheap_el) * heap->capacity);
	heap->size = 0;
	return heap;
}

void destroy_minheap(struct minheap* heap) {
	if (heap)
		free(heap->mem);
	free(heap);
}

static void minheap_swap(struct minheap* heap, int idx1, int idx2) {
	struct minheap_el t = heap->mem[idx1];
	heap->mem[idx1] = heap->mem[idx2];
	heap->mem[idx2] = t;
}

static void minheap_bubble_up(struct minheap* heap, int idx) {
	int idx_parent = (idx - 1) / 2;
	while (idx != 0 && heap->mem[idx].key < heap->mem[idx_parent].key) {
		minheap_swap(heap, idx, idx_parent);
		idx = idx_parent;
		idx_parent = (idx - 1) / 2;
	}
}

static void minheap_bubble_down(struct minheap* heap, int idx) {
	int key_this = heap->mem[idx].key;
	bool done = false;
	while (!done) {
		done = true;
		int smallest_key = key_this;
		int idx_smallest = idx;
		// find smallest between idx, left and right child
		int idx_child;
		for (idx_child = (idx * 2) + 1; idx_child <= (idx * 2) + 2; ++idx_child) {
			if (idx_child < heap->size) {
				int k = heap->mem[idx_child].key;
				if (k < smallest_key) {
					smallest_key = k;
					idx_smallest = idx_child;
				}
			}
		}
		if (idx_smallest != idx) {
			done = false;
			minheap_swap(heap, idx, idx_smallest);
			idx = idx_smallest;
		}
	}
}

void minheap_insert(struct minheap* heap, int key, int val) {
	// check capacity
	if (heap->size == heap->capacity) {
		heap->capacity = 2 * heap->capacity + 1; // 1 more layer
		heap->mem = realloc(heap->mem, sizeof(struct minheap_el) * heap->capacity);
	}
	int idx = heap->size; // insertion idx
	// copy info into new last element
	heap->mem[idx].key = key;
	heap->mem[idx].val = val;
	++heap->size;
	minheap_bubble_up(heap, idx);
}

int minheap_extract(struct minheap* heap, int* val) { // returns key, copies val to *val
	if (heap->size == 0) {
		fprintf(stderr, "Error: extracting from empty heap\n");
		exit(1);
	}
	int retKey = heap->mem[0].key;
	if (val)
		*val = heap->mem[0].val;

	--heap->size;
	if (heap->size > 0) {
		// copy ex-last element to root position
		heap->mem[0] = heap->mem[heap->size];
		minheap_bubble_down(heap, 0);
	}
	return retKey;
}

bool minheap_contains_val(struct minheap* heap, int val) {
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val)
			return true;
	}
	return false;
}

bool minheap_update_key_of_val(struct minheap* heap, int key, int val) {
	// Note: only changes key of first val found that had a different key
	// returns whether change was made
	bool changed = false;
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val && heap->mem[idx].key != key) {
			int oldkey = heap->mem[idx].key;
			heap->mem[idx].key = key;
			changed = true;
			if (key < oldkey)
				minheap_bubble_up(heap, idx);
			else
				minheap_bubble_down(heap, idx);
		}
	}
	return changed;
}

/************************* END MINHEAP ***************************/

// Use Dijkstra since we have no good heuristic here (A-star + h = 0 --> Dijkstra)

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

// Use Dijkstra since we have no good heuristic here (A-star + h = 0 --> Dijkstra)
int hscore(struct grid* g, int x, int y) {
	return 0;
}

int astar(struct grid* g) {
	// Implemented as A-star search with h() == 0
	// nodes are (x,y), encoded in integer:
	// y * w + x  (0<=y<h, 0<=x<w)
	int w = g->gridw;
	int h = g->gridh;
	int targetidx = g->targetidx;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
	}

	int idx = g->startidx;
	gscore[idx] = 0;
	//fscore[idx] = hscore(g, g->startpos.x, g->startpos.y)
	fscore = hscore(g, idx % w, idx / w);

	minheap_insert(open_set, fscore, idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		if (idx == targetidx)
			cost = gscore[idx]; // we're done here!
		else {
			// for each neighbor of current
			for (int dir = 0; dir < 4; ++dir) { // 4 neighboring cells
				int nx = x + dir2dx[dir];
				int ny = y + dir2dy[dir];
				if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
					int nidx = ny * w + nx;
					// portal? Note: additional step!
					int portal = 0;
					if (g->grid[nidx].label) {
						nidx = g->grid[nidx].portalidx;
						nx = nidx % w;
						ny = nidx / w;
						portal = 1;
					}
					// check if we can travel to neighbor
					if (g->grid[nidx].content == FLOOR) {
						int tentative_gscore = gscore[idx] + 1 + portal;
						if (tentative_gscore < gscore[nidx]) {
							gscore[nidx] = tentative_gscore;
							fscore = tentative_gscore + hscore(g, nx, ny);
							if (!minheap_contains_val(open_set, nidx))
								minheap_insert(open_set, fscore, nidx);
							else
								minheap_update_key_of_val(open_set, fscore, nidx); // Nasty...
						}
					}
				}
			}
		}
	}

	destroy_minheap(open_set);
	return cost;
}

void delete_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

struct grid* lines2grid(char lines[][MAX_LINE_LEN], int nr_lines) {
	// Find maze borders, not including labels
	int firstline = 0;
	while (!strchr(lines[firstline], '#'))
		++firstline;
	int lastline = firstline;
	while (strchr(lines[lastline], '#'))
		++lastline;
	--lastline;
	int firstchar = INT_MAX;
	int lastchar = 0;
	for (int l = firstline; l <= lastline; ++l) {
		int first = strchr(lines[l], '#') - lines[l];
		int last = strrchr(lines[l], '#') - lines[l];
		firstchar = first < firstchar ? first : firstchar;
		lastchar = last > lastchar ? last : lastchar;
	}
	// create maze
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = lastchar - firstchar + 1;
	g->gridh = lastline - firstline + 1;
	g->grid = malloc(g->gridw * g->gridh * sizeof(struct cell));
	// copy maze
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int linenr = firstline + y;
			int charnr = firstchar + x;
			char c = lines[linenr][charnr];
			int idx = y * g->gridw + x;
			g->grid[idx].content = EMPTY;
			g->grid[idx].label = 0;
			if (c == '#')
				g->grid[idx].content = WALL;
			else if (c == '.') {
				g->grid[idx].content = FLOOR; // TODO: merge EMPTY and FLOOR?
				// Label attached? LAbels get high numbers, to simplify relabeling algo later
				if (linenr >= 2 && isalpha(lines[linenr - 1][charnr]))
					g->grid[idx].label = 1000 + 26 * (lines[linenr - 2][charnr] - 'A') + (lines[linenr - 1][charnr] - 'A');
				else if (linenr + 2 < nr_lines && isalpha(lines[linenr + 1][charnr]))
					g->grid[idx].label = 1000 + 26 * (lines[linenr + 1][charnr] - 'A') + (lines[linenr + 2][charnr] - 'A');
				else if (charnr >= 2 && isalpha(lines[linenr][charnr - 1]))
					g->grid[idx].label = 1000 + 26 * (lines[linenr][charnr - 2] - 'A') + (lines[linenr][charnr - 1] - 'A');
				else if (charnr + 2 < MAX_LINE_LEN && isalpha(lines[linenr][charnr + 1]))
					g->grid[idx].label = 1000 + 26 * (lines[linenr][charnr + 1] - 'A') + (lines[linenr][charnr + 2] - 'A');
			}
		}
	}
	// translate labels into (x,y)
	int labelcount = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii) {
		int label = g->grid[ii].label;
		if (label == LABEL_AA) {
			g->startidx = ii;
			g->grid[ii].label = 0; // we no longer need this. Labels are portals
		}
		else if (label == LABEL_ZZ) {
			g->targetidx = ii;
			g->grid[ii].label = 0; // we no longer need this. Labels are portals
		}
		else if (label > LABEL_AA) { // find other end of portal, rename both
			++labelcount;
			for (int jj = ii + 1; jj < g->gridw * g->gridh; ++jj) {
				if (g->grid[jj].label == label) {
					g->grid[ii].portalidx = jj;
					g->grid[jj].portalidx = ii;
					// relabel portals
					g->grid[ii].label = labelcount;
					g->grid[jj].label = labelcount;
				}
			}
		}
	}

	return g;
}

void print_grid(struct grid* g) {
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		if (g->grid[ii].label != 0)
			printf("%d\n", g->grid[ii].label);

	// print
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			struct cell* c = &g->grid[idx];
			if (c->content == WALL)
				printf("#");
			else if (c->content == EMPTY)
				printf(" ");
			else { // FLOOR
				if (c->label == 0) {
					if (idx == g->startidx)
						printf("A");
					else if (idx == g->targetidx)
						printf("Z");
					else
						printf(".");
				}
				else
					printf("%c", 'a' + c->label - 1);
			}
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	// read all line and remember them
	char lines[MAX_LINES][MAX_LINE_LEN];
	int nr_lines = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		if (!empty_line(line))
			strcpy(lines[nr_lines++], line);
	free(line);

	struct grid* g = lines2grid(lines, nr_lines);

	//print_grid(g);
	int cost = astar(g);
	printf("%d\n", cost);

	delete_grid(g);

	return 0;
}
