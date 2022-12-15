#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define ABS(x) ((x)<0?(-(x)):(x))

struct cell {
	int height;
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
			c = 'a';
		}
		else if (c == 'E') {
			g->endpos.x = col;
			g->endpos.y = g->h;
			c = 'z';
		}
		g->grid[col + g->h * g->w].height = c - 'a';
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x].height + 'a';
			if (g->startpos.x == x && g->startpos.y == y)
				c = 'S';
			else if (g->endpos.x == x && g->endpos.y == y)
				c = 'E';
			printf("%c", c);
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

int hscore(struct grid* g, int x, int y) {
	int dx = g->endpos.x - x;
	int dy = g->endpos.y - y;
	return ABS(dx) + ABS(dy);
}

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

int find_target(struct grid* g) {
	// A-star search algo
	// nodes are (x,y), encoded in integer:
	// y * w + x  (0<=y<h, 0<=x<w)
	int w = g->w;
	int h = g->h;
	int target_idx = g->endpos.y * w + g->endpos.x;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	//int* fscore = malloc(w * h * sizeof(int)); // No need since it is key of open_set
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
		//fscore[ii] = INT_MAX;
	}

	int idx = g->startpos.y * w + g->startpos.x; // current state
	gscore[idx] = 0;
	//fscore[idx] = hscore(g, g->startpos.x, g->startpos.y)
	fscore = hscore(g, g->startpos.x, g->startpos.y);

	minheap_insert(open_set, fscore, idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		if (idx == target_idx)
			cost = gscore[idx]; // we're done here!
		else {
			// for each neighbor of current
			for (int dir = 0; dir < 4; ++dir) { // 4 neighboring cells
				int nx = x + dir2dx[dir];
				int ny = y + dir2dy[dir];
				if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
					int nidx = ny * w + nx;
					// check if we can travel to neighbor
					int dh = g->grid[nidx].height - g->grid[idx].height;
					if (dh <= 1) {
						int tentative_gscore = gscore[idx] + 1;
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


int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	//show_grid(grid);
	int cost = find_target(grid);
	printf("%d\n", cost);

	destroy_grid(grid);
	return 0;
}
