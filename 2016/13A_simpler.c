#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define SHOW_PATH

/************************* MINHEAP *******************************/
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

static void minheap_bubble_down(struct minheap* heap) {
	int idx = 0;
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
		minheap_bubble_down(heap);
	}
	return retKey;
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
void show_path(int startx, int starty, int targetx, int targety, bool* grid, int* camefrom, int gridw, int gridh) {
	int x, y;
	int idx;
	int startidx = starty * gridw + startx;

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
	int idx;

	// init
	int gridw = 2 * targetx;
	int gridh = 2 * targety; // TODO: if we go out of grid: redo with larger grid
	int targetidx = targety * gridw + targetx;

	bool* grid = malloc(gridw * gridh * sizeof(bool));
	int* gScore = malloc(gridw * gridh * sizeof(int));
#ifdef SHOW_PATH
	int* camefrom = malloc(gridw * gridh * sizeof(int));
#endif
	for (x = 0; x < gridw; ++x) {
		for (y = 0; y < gridh; ++y) {
			idx = y * gridw + x;
			grid[idx] = is_wall(x, y, key); // TODO: box around grid to avoid going off-grid?
			gScore[idx] = INT_MAX;
		}
	}
	struct minheap* heap = create_minheap();

	x = startx;
	y = starty;
	idx = y * gridw + x;
	gScore[idx] = 0;
	int fScore = hScore(x, y, targetx, targety);
	minheap_insert(heap, fScore, idx);

	bool found = false;
	while (heap->size) {
		// extract node with lowest fScore
		minheap_extract(heap, &idx); // idx = cur
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
					minheap_insert(heap, fScore, nidx);
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
	destroy_minheap(heap);
}

int main(int argc, char* argv[]) {
	// draw_grid(10, 10, 10, 7, 4); // test example
	// draw_grid(50, 50, 1364, 31, 39);

	// find_path(1, 1, 7, 4, 10); // test example
	find_path(1, 1, 31, 39, 1364);
	return 0;
}
