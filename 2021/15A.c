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
	// Dijkstra
	bool* visited = mycalloc(g->w * g->h, sizeof(bool));
	int*  cost    = mycalloc(g->w * g->h, sizeof(int)); // cost 0 means infinity here
	// init
	cost[0] = g->grid[0];
	int current_row = 0;
	int current_col = 0;
	int debug_cnt = 0;
	int target_idx = (g->h - 1) * g->w + (g->w - 1);
	while (!visited[target_idx] && debug_cnt++ < 100000) {
		// printf("Now at (%d, %d)\n", current_row, current_col);
		int idx = current_row * g->w + current_col;
		int c = cost[idx];
		if (current_col > 0 && !visited[idx - 1]) {
			if (cost[idx - 1] == 0 || c + g->grid[idx - 1] < cost[idx - 1]) {
				cost[idx - 1] = c + g->grid[idx - 1];
				// printf("Setting cost of (%d, %d) to %d\n", current_row, current_col - 1, cost[idx - 1]);
			}
		}
		if (current_row > 0 && !visited[idx - g->w]) {
			if (cost[idx - g->w] == 0 || c + g->grid[idx - g->w] < cost[idx - g->w]) {
				cost[idx - g->w] = c + g->grid[idx - g->w];
				// printf("Setting cost of (%d, %d) to %d\n", current_row - 1, current_col, cost[idx - DIM]);
			}
		}
		if (current_col < g->w - 1 && !visited[idx + 1]) {
			if (cost[idx + 1] == 0 || c + g->grid[idx + 1] < cost[idx + 1]) {
				cost[idx + 1] = c + g->grid[idx + 1];
				// printf("Setting cost of (%d, %d) to %d\n", current_row, current_col + 1, cost[idx + 1]);
			}
		}
		if (current_row < g->h - 1 && !visited[idx + g->w]) {
			if (cost[idx + g->w] == 0 || c + g->grid[idx + g->w] < cost[idx + g->w]) {
				cost[idx + g->w] = c + g->grid[idx + g->w];
				// printf("Setting cost of (%d, %d) to %d\n", current_row + 1, current_col, cost[idx + DIM]);
			}
		}
		visited[idx] = true;
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
	int cost = min_path_sum(&grid) - grid.grid[0]; // do not count upper left corner
	printf("%d\n", cost);

	free(line);
	return 0;
}
