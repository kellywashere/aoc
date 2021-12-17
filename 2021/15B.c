#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

void* mycalloc(size_t nmemb, size_t size) {
	void* p = calloc(nmemb, size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

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
	struct minheap* heap = mymalloc(sizeof(struct minheap));
	heap->capacity = 127; // 7 layers start size
	heap->mem = mymalloc(sizeof(struct minheap_el) * heap->capacity);
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


bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

struct grid {
	int  w;
	int  h;
	int* grid;
	int  capacity;
};

void grid_add_row(struct grid* g, const char* line) {
	int idx_line = 0;
	while (line[idx_line] >= '0' && line[idx_line] <= '9')
		++idx_line;
	if (g->grid != NULL && idx_line != g->w) {
		fprintf(stderr, "Inconsistent grid width %d != %d\n", idx_line, g->w);
		return;
	}
	if (g->grid == NULL) {
		g->w = idx_line;
		g->h = 0;
		g->capacity = g->w * g->w; // assume square
		g->grid = mymalloc(g->capacity * sizeof(int));
	}
	else if (g->w * (g->h + 1) > g->capacity) {
		g->capacity *= 2;
		g->grid = myrealloc(g->grid, g->capacity * sizeof(int));
	}
	int idx_grid = g->h * g->w;
	for (idx_line = 0; idx_line < g->w; ++idx_line)
		g->grid[idx_grid++] = line[idx_line] - '0';
	++g->h;
}

void print_grid(struct grid* g) {
	int r, c;
	for (r = 0; r < g->h; ++r) {
		for (c = 0; c < g->w; ++c) {
			putchar(g->grid[r * g->w + c] + '0');
		}
		putchar('\n');
	}
}

int min_path_sum(struct grid* g) {
	int nr_nodes = g->w * g->h;
	int nr_nodes_visited = 0;
	// Dijkstra
	bool* visited = mycalloc(nr_nodes, sizeof(bool));
	int*  cost    = mycalloc(nr_nodes, sizeof(int)); // cost 0 means infinity here
	// init
	cost[0] = g->grid[0];
	int current_row = 0;
	int current_col = 0;
	int target_idx = (g->h - 1) * g->w + (g->w - 1);
	while (!visited[target_idx]) {
		// printf("Now at (%d, %d)\n", current_row, current_col);
		int idx = current_row * g->w + current_col;
		int c = cost[idx];
		if (current_col > 0 && !visited[idx - 1]) {
			if (cost[idx - 1] == 0 || c + g->grid[idx - 1] < cost[idx - 1]) {
				cost[idx - 1] = c + g->grid[idx - 1];
			}
		}
		if (current_row > 0 && !visited[idx - g->w]) {
			if (cost[idx - g->w] == 0 || c + g->grid[idx - g->w] < cost[idx - g->w]) {
				cost[idx - g->w] = c + g->grid[idx - g->w];
			}
		}
		if (current_col < g->w - 1 && !visited[idx + 1]) {
			if (cost[idx + 1] == 0 || c + g->grid[idx + 1] < cost[idx + 1]) {
				cost[idx + 1] = c + g->grid[idx + 1];
			}
		}
		if (current_row < g->h - 1 && !visited[idx + g->w]) {
			if (cost[idx + g->w] == 0 || c + g->grid[idx + g->w] < cost[idx + g->w]) {
				cost[idx + g->w] = c + g->grid[idx + g->w];
			}
		}
		visited[idx] = true;
		++nr_nodes_visited;
		if (nr_nodes_visited % 1000 == 0)
			printf("Nodes visited: %d\n", nr_nodes_visited);
		// find unvisited node with lowest cost
		c = 0;
		int row, col;
		for (row = 0; row < g->h; ++row) {
			for (col = 0; col < g->w; ++col) {
				idx = row * g->w + col;
				if (!visited[idx] && (cost[idx] > 0 && (c == 0 || cost[idx] < c))) {
					current_row = row;
					current_col = col;
					c = cost[idx];
				}
			}
		}
	}
	return cost[target_idx];
}

struct grid* expand_grid(struct grid* g) {
	struct grid* g25 = mymalloc(sizeof(struct grid));
	g25->w = 5 * g->w;
	g25->h = 5 * g->h;
	g25->capacity = g25->w * g25->h;
	g25->grid = mymalloc(g25->capacity * sizeof(int));
	int r, c;
	for (r = 0; r < g25->h; ++r) {
		int rr = r % g->h;
		for (c = 0; c < g25->w; ++c) {
			int cc = c % g->w;
			int v = g->grid[rr * g->w + cc];
			v += (r / g->h) + (c / g->w);
			v = (v - 1) % 9 + 1;
			g25->grid[r * g25->w + c] = v;
		}
	}
	return g25;
}

int main(int argc, char* argv[]) {
	struct grid grid;
	grid.grid = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		grid_add_row(&grid, line);
	}
	struct grid* gbig = expand_grid(&grid);
	int cost = min_path_sum(gbig) - gbig->grid[0]; // do not count upper left corner
	printf("%d\n", cost);

	free(line);

	// clean up
	free(grid.grid);
	free(gbig->grid);
	free(gbig);
	return 0;
}
