#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#include <assert.h>

#define ABS(x) ((x)<0?(-(x)):(x))

struct cell {
	bool iswall;
};

struct vec2 {
	int x;
	int y;
};

/////////////////////// Vec2 Array /////////////////////////////////////

struct vec2_array {
	size_t       size;
	size_t       capacity;
	struct vec2* data;
};

struct vec2_array* vec2_array_create() {
	struct vec2_array* v2a = malloc(sizeof(struct vec2_array));
	v2a->size = 0;
	v2a->capacity = 256;
	v2a->data = malloc(v2a->capacity * sizeof(struct vec2));
	return v2a;
}

void vec2_array_destroy(struct vec2_array* v2a) {
	if (v2a) {
		free(v2a->data);
		free(v2a);
	}
}

size_t vec2_array_size(struct vec2_array* v2a) {
	return v2a->size;
}

struct vec2 vec2_array_get(struct vec2_array* v2a, int idx) {
	assert(idx < v2a->size);
	struct vec2 v = v2a->data[idx];
	return v;
}

void vec2_array_push(struct vec2_array* v2a, struct vec2 v) {
	if (v2a->size == v2a->capacity) {
		v2a->capacity *= 2;
		v2a->data = realloc(v2a->data, v2a->capacity * sizeof(struct vec2));
	}
	v2a->data[v2a->size++] = v;
}

struct vec2 vec2_array_pop(struct vec2_array* v2a) {
	assert(v2a->size > 0);
	--v2a->size;
	return v2a->data[v2a->size];
}


//////////////////////////////////// Grid ////////////////////////////

struct grid {
	int           w;
	int           h;
	struct cell*  grid;

	struct vec2   startpos;
	struct vec2   endpos;
};

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = w;
	g->h = h;
	g->grid = calloc(w * h, sizeof(struct cell));
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
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
int hscore(struct grid* g, int x, int y) {
	int h = ABS(x - g->endpos.x) + ABS(y - g->endpos.y);
	return h;
}

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

int find_target(struct grid* g) {
	// A-star search algo
	// nodes are (x,y), encoded in integer:
	// y * w + x  (0<=y<h, 0<=x<w)
	int w = g->w;
	int h = g->h;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	//int* fscore = malloc(w * h * sizeof(int)); // No need since it is key of open_set
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
		//fscore[ii] = INT_MAX;
	}

	int idx = g->startpos.y * w + g->startpos.x;
	gscore[idx] = 0;
	//fscore[idx] = hscore(g, g->startpos.x, g->startpos.y)
	fscore = hscore(g, g->startpos.x, g->startpos.y);

	minheap_insert(open_set, fscore, idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		if (x == g->endpos.x && y == g->endpos.y) {
			cost = gscore[idx]; // we're done here!
		}
		else {
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

bool read_int(const char** pLine, int* x) {
	int num = 0;
	const char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}


int main(int argc, char* argv[]) {
	int w = argc > 1 ? atoi(argv[1]) : 71;
	int h = argc > 2 ? atoi(argv[2]) : 71;
	struct grid* grid = create_grid(w, h);
	grid->startpos = (struct vec2){0, 0};
	grid->endpos   = (struct vec2){grid->w - 1, grid->h - 1};

	struct vec2_array* vec2s = vec2_array_create();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int x, y;
		const char* l = line;
		read_int(&l, &x);
		read_int(&l, &y);
		vec2_array_push(vec2s, (struct vec2){x, y});
	}
	free(line);

	// We will place m walls. We look for lowest m where no path is found
	int low = 0;
	int high = vec2_array_size(vec2s);
	// low < m <= high
	while (low < high - 1) {
		int m = (low + high + 1) / 2;
		for (int ii = 0; ii < grid->w * grid->h; ++ii)
			grid->grid[ii].iswall = false;
		for (int ii = 0; ii < m; ++ii) {
			struct vec2 v = vec2_array_get(vec2s, ii);
			grid->grid[v.x + v.y * grid->w].iswall = true;
		}
		int cost = find_target(grid);
		if (cost < INT_MAX) { // path found
			low = m;
		}
		else { // no path found
			high = m;
		}
	}
	// now: m = high
	struct vec2 v = vec2_array_get(vec2s, high - 1); // index of last placed wall is high-1
	printf("%d,%d\n", v.x, v.y);

	destroy_grid(grid);
	return 0;
}
