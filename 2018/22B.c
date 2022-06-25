#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MOD 20183

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

void read_inputs(int* depth, int* targetx, int* targety) {
	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	char* l = line;
	*depth = read_int(&l);

	getline(&line, &len, stdin);
	l = line;
	*targetx = read_int(&l);
	*targety = read_int(&l);
	free(line);
}

enum type {
	ROCKY  = 0,
	WET    = 1,
	NARROW = 2
};

enum tool { /* numbered s.t. if tool == type --> tool not allowed */
	NOTOOL = 0,
	TORCH  = 1,
	CLIMBGEAR = 2
};

struct cell {
	int       geoidx;
	enum type type;
};

struct grid {
	int          gridw;
	int          gridh;
	int          targetx;
	int          targety;
	int          depth;
	struct cell* grid;
};

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = w;
	g->gridh = h;
	g->grid = calloc(w * h, sizeof(struct cell));
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void calc_geoidx_and_type(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			int geoidx;
			if (idx == 0)
				geoidx = 0;
			else if (x == g->targetx && y == g->targety)
				geoidx = 0;
			else if (y == 0)
				geoidx = (x * 16807) % MOD;
			else if (x == 0)
				geoidx = (y * 48271) % MOD;
			else {
				int el1 = (g->grid[idx - 1].geoidx + g->depth) % MOD;
				int el2 = (g->grid[idx - g->gridw].geoidx + g->depth) % MOD;
				geoidx = (el1 * el2) % MOD;
			}
			g->grid[idx].geoidx = geoidx;
			int el = ((geoidx + g->depth) % MOD);
			g->grid[idx].type = el % 3;
		}
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

#define SWAP_COST 7

int hscore(struct grid* map, int x, int y, enum tool tool) {
	int dx = map->targetx - x;
	int dy = map->targety - y;
	dx = dx < 0 ? -dx : dx;
	dy = dy < 0 ? -dy : dy;
	return dx + dy + (tool != TORCH ? SWAP_COST : 0);
}

int dir2dx[]         = {1, 0, -1, 0, 0};
int dir2dy[]         = {0, -1, 0, 1, 0};
int dir2toolchange[] = {0, 0,  0, 0, 1};
int dir2cost[]       = {1, 1,  1, 1, SWAP_COST};

int find_target(struct grid* map) {
	// A-star search algo
	// a nodes are (x,y,tool), encoded in integer:
	// 3*(y * w + x) + t  (0<=y<h, 0<=x<w, 0<=t<3)
	int w = map->gridw;
	int h = map->gridh;
	int target_idx = 3*(map->targety * w + map->targetx) + TORCH;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * 3 * sizeof(int));
	int* fscore = malloc(w * h * 3 * sizeof(int));
	for (int ii = 0; ii < w * h * 3; ++ii) {
		gscore[ii] = INT_MAX;
		fscore[ii] = INT_MAX;
	}

	int idx = 3*(0 * w + 0) + TORCH; // current state (0,0, TORCH)
	gscore[idx] = 0;
	fscore[idx] = hscore(map, 0, 0, TORCH);

	minheap_insert(open_set, fscore[idx], idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int tool = idx % 3;
		int mapidx = idx / 3; // idx in map->grid[]
		int y = mapidx / w;
		int x = mapidx % w;
		int type = map->grid[mapidx].type;
		if (idx == target_idx)
			cost = gscore[idx]; // we're done here!
		else {
			// for each neighbor of current
			for (int dir = 0; dir < 5; ++dir) { // 4 neighboring cells, and 1 tool change = 5
				int ntool = tool;
				if (dir2toolchange[dir])
					ntool = 3 - tool - type; // tool & type are encoded s.t. allowed tool1 + allowed tool2 + type == 3
				int nx = x + dir2dx[dir];
				int ny = y + dir2dy[dir];
				if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
					int nmapidx = ny * w + nx;
					int nidx = 3*nmapidx + ntool;
					// check if we can travel with current tool
					if (map->grid[nmapidx].type != ntool) { // possible thanks to encoding of type & tool
						int tentative_gscore = gscore[idx] + dir2cost[dir];
						if (tentative_gscore < gscore[nidx]) {
							gscore[nidx] = tentative_gscore;
							fscore[nidx] = tentative_gscore + hscore(map, nx, ny, ntool);
							if (!minheap_contains_val(open_set, nidx))
								minheap_insert(open_set, fscore[nidx], nidx);
							else
								minheap_update_key_of_val(open_set, fscore[nidx], nidx); // Nasty...
						}
					}
				}
			}
		}
	}

	destroy_minheap(open_set);
	return cost;
}

void show_grid_type(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			int type = g->grid[idx].type;
			putchar(type == 0 ? '.' : type == 1 ? '=' : '|');
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[]) {
	int depth = 0;
	int targetx = 0;
	int targety = 0;
	read_inputs(&depth, &targetx, &targety);

	struct grid* grid = create_grid(targetx + 1 + targetx, targety + 1 + targety); // target in middle
	grid->depth = depth;
	grid->targetx = targetx;
	grid->targety = targety;

	calc_geoidx_and_type(grid);

	// show_grid_type(grid);

	// puzzle answer:
	int cost = find_target(grid);
	printf("%d\n", cost);

	destroy_grid(grid);
	return 0;
}
