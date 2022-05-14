#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define SHOW_PATH

/************************* MINHEAP *******************************/
struct minheap_el {
	int          key;
	unsigned int val;
};

struct minheap_vallut { // minheap, with lut for val-->idx translation to find val in heap with O(1)
	// NOTE: this structure is a set as far as val is concerned. One value can appear only once in structure
	struct minheap_el* mem;      // array-based min heap
	unsigned int       size;     // nr elements currently in heap
	unsigned int       capacity; // capacity in nr of elements
	unsigned int       maxval;   // maximum value of val  TODO: hash table does not need this
	unsigned int*      vallut;   // vallut[val] gives idx in mem where this val is stored  TODO: hash table
};

struct minheap_vallut* create_minheap_vallut(unsigned int maxval) {
	struct minheap_vallut* heap = malloc(sizeof(struct minheap_vallut));
	heap->capacity = 127; // 7 layers start size
	heap->mem = malloc(sizeof(struct minheap_el) * heap->capacity);
	heap->size = 0;
	heap->maxval = maxval;
	heap->vallut = malloc((maxval + 1) * sizeof(int));
	for (unsigned int ii = 0; ii <= maxval; ++ii)
		heap->vallut[ii] = -1;
	return heap;
}

void destroy_minheap_vallut(struct minheap_vallut* heap) {
	if (heap) {
		free(heap->mem);
		free(heap->vallut);
		free(heap);
	}
}

static void minheap_vallut_swap(struct minheap_vallut* heap, int idx1, int idx2) {
	struct minheap_el t = heap->mem[idx1];
	heap->mem[idx1] = heap->mem[idx2];
	heap->mem[idx2] = t;
	heap->vallut[heap->mem[idx1].val] = idx1;
	heap->vallut[heap->mem[idx2].val] = idx2;
}

static unsigned int minheap_vallut_bubble_up(struct minheap_vallut* heap, unsigned int idx) {
	// returns new idx of bubbled-up item
	unsigned int idx_parent = (idx - 1) / 2;
	while (idx != 0 && heap->mem[idx].key < heap->mem[idx_parent].key) {
		minheap_vallut_swap(heap, idx, idx_parent);
		idx = idx_parent;
		idx_parent = (idx - 1) / 2;
	}
	return idx;
}

static unsigned int minheap_vallut_bubble_down(struct minheap_vallut* heap, unsigned int idx) {
	int key_this = heap->mem[idx].key;
	bool done = false;
	while (!done) {
		done = true;
		int smallest_key = key_this;
		unsigned int idx_smallest = idx;
		// find smallest between idx, left and right child
		unsigned int idx_child;
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
			minheap_vallut_swap(heap, idx, idx_smallest);
			idx = idx_smallest;
		}
	}
	return idx;
}

void minheap_vallut_insert(struct minheap_vallut* heap, int key, unsigned int val) {
	if (heap->vallut[val] != -1) {
		fprintf(stderr, "Error: value %d already appears in heap\n", val);
		exit(1);
	}
	// check capacity
	if (heap->size == heap->capacity) {
		heap->capacity = 2 * heap->capacity + 1; // 1 more layer
		heap->mem = realloc(heap->mem, sizeof(struct minheap_el) * heap->capacity);
	}
	unsigned int idx = heap->size; // insertion idx
	// copy info into new last element
	heap->mem[idx].key = key;
	heap->mem[idx].val = val;
	heap->vallut[val] = idx;
	++heap->size;
	minheap_vallut_bubble_up(heap, idx);
}

int minheap_vallut_extract(struct minheap_vallut* heap, unsigned int* val) { // returns key, copies val to *val
	if (heap->size == 0) {
		fprintf(stderr, "Error: extracting from empty heap\n");
		exit(1);
	}
	int retKey = heap->mem[0].key;
	if (val)
		*val = heap->mem[0].val;
	heap->vallut[heap->mem[0].val] = -1;

	--heap->size;
	if (heap->size > 0) {
		// copy ex-last element to root position
		heap->mem[0] = heap->mem[heap->size];
		heap->vallut[heap->mem[0].val] = 0;
		minheap_vallut_bubble_down(heap, 0);
	}
	return retKey;
}

bool minheap_vallut_hasvalue(struct minheap_vallut* heap, unsigned int val) {
	return heap->vallut[val] != -1;
}

void minheap_vallut_changekey(struct minheap_vallut* heap, int key, unsigned int val) {
	unsigned int idx = heap->vallut[val];
	if (idx == -1) {
		fprintf(stderr, "Error: value %d does not appear in heap\n", val);
		exit(1);
	}
	heap->mem[idx].key = key;
	idx = minheap_vallut_bubble_up(heap, idx);
	minheap_vallut_bubble_down(heap, idx);
}
/************************* MINHEAP *******************************/

int popcount(int x) {
	int c = 0;
	while (x) {
		c += x & 1;
		x >>= 1;
	}
	return c;
}

bool is_wall(int x, int y, int key) {
	int n = x*x + 3*x + 2*x*y + y + y*y;
	n += key;
	return popcount(n) % 2 == 1;
}

void draw_grid(int w, int h, int key, int targetx, int targety) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			bool is_target = x == targetx && y == targety;
			printf("%c", is_target ? 'X' : is_wall(x, y, key) ? '#' : '.');
		}
		printf("\n");
	}
}

int hScore(int x, int y, int targetx, int targety) {
	int dx = x - targetx;
	dx = dx < 0 ? -dx : dx;
	int dy = y - targety;
	dy = dy < 0 ? -dy : dy;
	return dx + dy;
}

enum dir {
	STATIONARY,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NR_DIRS
};

#ifdef SHOW_PATH
enum dir didx_to_dir(int didx) {
	return didx == 1 ? RIGHT : (didx == -1 ? LEFT : (didx > 0 ? DOWN : UP));
}
#endif

int dir_to_didx(enum dir dir, int gridw) {
	switch(dir) {
		case UP:
			return -gridw;
		case DOWN:
			return gridw;
		case LEFT:
			return -1;
		case RIGHT:
			return 1;
		default:
			return 0;
	}
}

#ifdef SHOW_PATH
void show_path(int startx, int starty, int targetx, int targety, bool* grid, enum dir* camefrom, int gridw, int gridh) {
	int x, y;
	unsigned int idx;
	unsigned int startidx = starty * gridw + startx;

	int* solgrid = malloc(gridw * gridh * sizeof(int));
	for (idx = 0; idx < gridw * gridh; ++idx)
		solgrid[idx] = STATIONARY;
	// backtrace from target to start
	idx = targety * gridw + targetx;
	while (idx != startidx) {
		int nextidx = camefrom[idx];
		solgrid[nextidx] = didx_to_dir(idx - nextidx);
		idx = nextidx;
	}
	for (y = 0; y < gridh; ++y) {
		for (x = 0; x < gridw; ++x) {
			idx = y * gridw + x;
			char c = grid[idx] ? '#' :
				     (x == targetx && y == targety) ? 'X' :
				     solgrid[idx] == UP ? '^' :
				     solgrid[idx] == DOWN ? 'v' :
				     solgrid[idx] == LEFT ? '<' :
				     solgrid[idx] == RIGHT ? '>' : ' ';
			printf("%c", c);
		}
		printf("\n");
	}

	free(solgrid);
}
#endif

void find_path(int startx, int starty, int targetx, int targety, int key) {
	int x, y;
	unsigned int idx;

	// init
	int gridw = 2 * targetx;
	int gridh = 2 * targety; // TODO: if we go out of grid: redo with larger grid
	int targetidx = targety * gridw + targetx;

	bool* grid = malloc(gridw * gridh * sizeof(bool));
	int* gScore = malloc(gridw * gridh * sizeof(int));
#ifdef SHOW_PATH
	unsigned int* camefrom = malloc(gridw * gridh * sizeof(unsigned int));
#endif
	for (x = 0; x < gridw; ++x) {
		for (y = 0; y < gridh; ++y) {
			idx = y * gridw + x;
			grid[idx] = is_wall(x, y, key); // TODO: box around grid to avoid going off-grid?
			gScore[idx] = INT_MAX;
		}
	}
	struct minheap_vallut* heap = create_minheap_vallut(gridw * gridh - 1);

	x = startx;
	y = starty;
	idx = y * gridw + x;
	gScore[idx] = 0;
	int fScore = hScore(x, y, targetx, targety);
	minheap_vallut_insert(heap, fScore, idx);

	bool found = false;
	while (heap->size) {
		// extract node with lowest fScore
		minheap_vallut_extract(heap, &idx); // idx = cur
		if (idx == targetidx) {
			found = true;
			break;
		}
		for (enum dir dir = UP; dir < NR_DIRS; ++dir) {
			int nidx = idx + dir_to_didx(dir, gridw);
			if (!grid[nidx]) {
				int tentative_gScore = gScore[idx] + 1;
				if (tentative_gScore < gScore[nidx]) { // found better path to nidx node
#ifdef SHOW_PATH
					camefrom[nidx] = idx;
#endif
					gScore[nidx] = tentative_gScore;
					int h = hScore(nidx % gridw, nidx / gridw, targetx, targety);
					fScore = tentative_gScore + h;
					if (minheap_vallut_hasvalue(heap, nidx))
						minheap_vallut_changekey(heap, fScore, nidx);
					else
						minheap_vallut_insert(heap, fScore, nidx);
				}
			}
		}
	}
	if (found) {
#ifdef SHOW_PATH
		show_path(startx, starty, targetx, targety, grid, camefrom, gridw, gridh);
#endif
		printf("Path len: %d\n", gScore[targetidx]);
	}

	free(grid);
	free(gScore);
#ifdef SHOW_PATH
	free(camefrom);
#endif
	destroy_minheap_vallut(heap);
}

int main(int argc, char* argv[]) {
	// draw_grid(10, 10, 10, 7, 4); // test example
	// draw_grid(50, 50, 1364, 31, 39);

	// find_path(1, 1, 7, 4, 10); // test example
	find_path(1, 1, 31, 39, 1364);
	return 0;
}
