#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

#define ABS(x) ((x)<0?(-(x)):(x))

struct cell {
	bool iswall;
};

struct vec2 {
	int x;
	int y;
};

struct grid {
	int           w;
	int           h;
	int           rowcapacity; // max nr of rows reserved in mem
	struct cell*  grid;

	struct vec2   startpos;
	struct vec2   endpos;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = 0;
	g->h = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (line[lw] && !isspace(line[lw]))
		++lw;
	if (g->w > 0 && g->w != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return;
	}
	if (!g->grid) {
		g->w = lw;
		g->h = 0;
		g->rowcapacity = lw; // assume square
		g->grid = calloc(g->w * g->rowcapacity, sizeof(struct cell));
	}
	if (g->h >= g->rowcapacity) {
		g->grid = realloc(g->grid, g->w * 2 * g->rowcapacity * sizeof(struct cell));
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		char c = line[col];
		if (c == 'S') {
			g->startpos.x = col;
			g->startpos.y = g->h;
		}
		else if (c == 'E') {
			g->endpos.x = col;
			g->endpos.y = g->h;
		}
		g->grid[col + g->h * g->w].iswall = c == '#';
	}
	++g->h;
}

void show_grid(struct grid* g, int* gscore) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			int idx = y * g->w + x;
			char c = g->grid[idx].iswall ? '#' : '.';
			if (g->startpos.x == x && g->startpos.y == y)
				printf(RED "S" COL_RESET);
			else if (g->endpos.x == x && g->endpos.y == y)
				printf(RED "E" COL_RESET);
			else {
				if (gscore && gscore[idx] < INT_MAX)
					printf(YEL "%d" COL_RESET, gscore[idx] % 10);
				else
					printf("%c", c);
			}
		}
		printf("\n");
	}
}

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

// Heuristic for A-star
int hscore(struct grid* g, int x, int y) {
	int h = ABS(x - g->endpos.x) + ABS(y - g->endpos.y);
	return h;
}

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

int* find_distances(struct grid* g) {
	// Dijkstra (we need to visit all nodes anyway, no need for heuristics)
	// nodes are (x,y), encoded in integer:
	// y * w + x  (0<=y<h, 0<=x<w)
	int w = g->w;
	int h = g->h;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
	}

	int idx = g->endpos.y * w + g->endpos.x; // current state: Endpos (reverse search!)
	gscore[idx] = 0;
	//fscore[idx] = hscore(g, g->startpos.x, g->startpos.y)
	fscore = 0; // g + h

	minheap_insert(open_set, fscore, idx);

	while (open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		// for each neighbor of current
		for (int dir = 0; dir < 4; ++dir) { // 4 neighboring cells
			int nx = x + dir2dx[dir];
			int ny = y + dir2dy[dir];
			if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
				int nidx = ny * w + nx;
				// check if we can travel to neighbor
				if (!g->grid[nidx].iswall) {
					int tentative_gscore = gscore[idx] + 1;
					if (tentative_gscore < gscore[nidx]) {
						gscore[nidx] = tentative_gscore;
						fscore = tentative_gscore;
						if (!minheap_contains_val(open_set, nidx))
							minheap_insert(open_set, fscore, nidx);
						else
							minheap_update_key_of_val(open_set, fscore, nidx); // Nasty...
					}
				}
			}
		}
	}

	destroy_minheap(open_set);
	return gscore;
}

int find_cheats(struct grid* g) {
	int count = 0;
	int* dist_to_end = find_distances(g);
	for (int y = 1; y < g->h - 1; ++y) { // avoid edge
		for (int x = 1; x < g->w - 1; ++x) {
			int idx = x + y * g->w;
			if (dist_to_end[idx] == INT_MAX)
				continue;
			// find any number that is 2 steps away with a distance delta to this cell >2
			for (int y2 = y - 20; y2 <= y + 20; ++y2) {
				if (y2 <= 0 || y2 >= g->h - 1) continue;
				for (int x2 = x - 20; x2 <= x + 20; ++x2) {
					if (x2 <= 0 || x2 >= g->w - 1) continue;
					int idx2 = x2 + y2 * g->w;
					if (dist_to_end[idx2] == INT_MAX) continue;
					int d = ABS(x2 - x) + ABS(y2 - y);
					if (d > 20) continue;
					int gain = dist_to_end[idx2] - dist_to_end[idx] - d;
					// if (gain > 0) {
					// 	printf("Cheat found: %d\n", gain);
					// }
					count += gain >= 100 ? 1 : 0; // problem cut-off
				}
			}
		}
	}

	free(dist_to_end);
	return count;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	int count = find_cheats(grid);
	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
