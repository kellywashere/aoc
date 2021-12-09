#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "Could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

struct grid {
	size_t nr_rows;
	size_t nr_cols;
	int**  rows; // each row is its own piece of memory
	size_t row_capacity; // max nr of rows
};

#define GRID_INIT_CAPACITY 16

struct grid* create_grid() {
	struct grid* g = mymalloc(sizeof(struct grid));
	g->row_capacity = GRID_INIT_CAPACITY;
	g->rows = mymalloc(g->row_capacity * sizeof(int*));
	g->nr_cols = 0;
	g->nr_rows = 0;
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		for (int ii = 0; ii < g->nr_rows; ++ii)
			free(g->rows[ii]);
		free(g);
	}
}

void grid_add_row(struct grid* g, char* rowstr) {
	if (g->nr_rows == g->row_capacity) {
		g->row_capacity *= 2;
		g->rows = myrealloc(g->rows, g->row_capacity * sizeof(int*));
	}
	int nr_cols = 0;
	while (rowstr[nr_cols] >= '0' && rowstr[nr_cols] <= '9')
		++nr_cols;
	if (g->nr_cols == 0)
		g->nr_cols = nr_cols;
	else if (g->nr_cols != nr_cols) {
		fprintf(stderr, "Inconsistent nr of colums\n");
		return;
	}
	int* newrow = mymalloc(nr_cols * sizeof(int));
	for (int col = 0; col < nr_cols; ++col)
		newrow[col] = rowstr[col] - '0';
	g->rows[g->nr_rows++] = newrow;
}

// debug:
void print_grid(struct grid* g) {
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			printf("%d", g->rows[row][col]);
		}
		printf("\n");
	}
}

int find_low_points(struct grid* g) {
	int sum = 0;
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			int t = g->rows[row][col];
			bool lowest = row == 0 || g->rows[row - 1][col] > t;
			lowest = lowest && (row == g->nr_rows - 1 || g->rows[row + 1][col] > t);
			lowest = lowest && (col == 0 || g->rows[row][col - 1] > t);
			lowest = lowest && (col == g->nr_cols - 1 || g->rows[row][col + 1] > t);
			if (lowest)
				sum += t + 1;
		}
	}
	return sum;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		grid_add_row(grid, line);
	}
	printf("%d\n", find_low_points(grid));
	destroy_grid(grid);
	free(line);
	return 0;
}
