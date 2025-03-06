#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define ABS(x) ((x)<0?(-(x)):(x))

#define MOVE_COST     1
#define ROTATION_COST 1000


struct cell {
	bool iswall;
};

struct vec2 {
	int x;
	int y;
};

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

enum dir {
	EAST,
	SOUTH,
	WEST,
	NORTH
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

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x].iswall ? '#' : '.';
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

// Heuristic for A-star
// TODO: give facing info as param
int hscore(struct grid* g, int x, int y) {
	int h = ABS(x - g->endpos.x) + ABS(x - g->endpos.y);
	h *= MOVE_COST;
	if (x != g->endpos.x && y != g->endpos.y) // at least one rotation needed
		h += ROTATION_COST;
	return 0;
}

#define NR_FACING 2 /* East-West, or North-South */
int find_target(struct grid* g) {
	// A-star search algo
	// nodes are (x,y,facing), encoded in integer:
	// 2 * (y * w + x) + facing  (0<=y<h, 0<=x<w)
	// facing is 0 when facing east or west, and 1 when facing north or south
	int w = g->w;
	int h = g->h;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(NR_FACING * w * h * sizeof(int)); // 2* for 'facing'
	//int* fscore = malloc(...); // No need since it is key of open_set
	int fscore;
	for (int ii = 0; ii < NR_FACING * w * h; ++ii) {
		gscore[ii] = INT_MAX;
		//fscore[ii] = INT_MAX;
	}

	int idx = 2 * (g->startpos.y * w + g->startpos.x) + 0; // current state, facing EAST
	gscore[idx] = 0;
	fscore = hscore(g, g->endpos.x, g->endpos.y); // g + h

	minheap_insert(open_set, fscore, idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = (idx / NR_FACING) / w;
		int x = (idx / NR_FACING) % w;
		int facing = idx % NR_FACING;

		// gen neighors
		// 3 neighbors: two without rotation, and rotaating (changing facing value)
		int dx = facing == 0 ? 1 : 0;
		int dy = facing == 1 ? 1 : 0;
		int xns[3] = { x - dx, x + dx, x  }; // x of neighbors
		int yns[3] = { y - dy, y + dy, y  }; // y of neighbors
		int dns[3] = { facing, facing, 1 - facing  }; // facing value of neighbors
		int cns[3] = { MOVE_COST, MOVE_COST, ROTATION_COST }; // cost of going to neighbor
		if (x == g->endpos.x && y == g->endpos.y)
			cost = gscore[idx]; // we're done here!
		else {
			// go over all neightbors
			for (int n = 0; n < 3; ++n) { // 4 neighboring cells
				if (xns[n] >= 0 && xns[n] < w && yns[n] >= 0 && yns[n] < h) {
					int grid_idx = yns[n] * w + xns[n];
					int nidx = 2 * grid_idx + dns[n]; // idx in gscore
					// check if we can travel to neighbor
					if (!g->grid[grid_idx].iswall) {
						int tentative_gscore = gscore[idx] + cns[n];
						if (tentative_gscore < gscore[nidx]) {
							gscore[nidx] = tentative_gscore;
							fscore = tentative_gscore + hscore(g, xns[n], yns[n]);
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

	// show_grid(grid);

	int cost = find_target(grid);
	printf("%d\n", cost);

	destroy_grid(grid);
	return 0;
}
