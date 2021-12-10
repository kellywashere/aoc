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

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
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
	g->nr_rows = 0;
	g->nr_cols = 0;
	return g;
}

struct grid* create_grid_with_size(int nr_rows, int nr_cols) {
	struct grid* g = mymalloc(sizeof(struct grid));
	g->row_capacity = nr_rows;
	g->rows = mymalloc(g->row_capacity * sizeof(int*));
	g->nr_rows = nr_rows;
	g->nr_cols = nr_cols;
	for (int r = 0; r < nr_rows; ++r) {
		g->rows[r] = mymalloc(nr_cols * sizeof(int));
		memset(g->rows[r], 0, nr_cols * sizeof(int));
	}
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
	while (rowstr[nr_cols] == '.' || rowstr[nr_cols] == '#')
		++nr_cols;
	if (g->nr_cols == 0)
		g->nr_cols = nr_cols;
	else if (g->nr_cols != nr_cols) {
		fprintf(stderr, "Inconsistent nr of colums\n");
		return;
	}
	int* newrow = mymalloc(nr_cols * sizeof(int));
	for (int col = 0; col < nr_cols; ++col)
		newrow[col] = rowstr[col] == '#' ? 1 : 0;
	g->rows[g->nr_rows++] = newrow;
}

// debug:
void print_grid(struct grid* g) {
	printf("\n");
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			printf("%c", g->rows[row][col] ? '#' : '.');
		}
		printf("\n");
	}
}

int count_neighbors(struct grid* g, int row, int col) {
	// Alternative to range checking is to have an extra rectangle
	// of dead cells / off lights around grid of interest
	int startrow = row > 0 ? row - 1 : 0;
	int endrow = row < g->nr_rows - 1 ? row + 1 : g->nr_rows - 1;
	int startcol = col > 0 ? col - 1 : 0;
	int endcol = col < g->nr_cols - 1 ? col + 1 : g->nr_cols - 1;
	int sum = 0;
	for (int r = startrow; r <= endrow; ++r)
		for (int c = startcol; c <= endcol; ++c)
			sum += g->rows[r][c];
	sum -= g->rows[row][col]; // row, col is not a neighbor
	return sum;
}

int count_lights(struct grid* g) {
	int count = 0;
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			count += g->rows[row][col];
		}
	}
	return count;
}

void step_grid(struct grid* g, struct grid* g2) {
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			int n = count_neighbors(g, row, col);
			if (g->rows[row][col])
				g2->rows[row][col] = (n == 2 || n == 3) ? 1 : 0;
			else
				g2->rows[row][col] = n == 3 ? 1 : 0;
		}
	}
	// 18B: turn on corner lights
	g2->rows[0][0] = 1;
	g2->rows[g->nr_rows - 1][0] = 1;
	g2->rows[g->nr_rows - 1][g->nr_cols - 1] = 1;
	g2->rows[0][g->nr_cols - 1] = 1;
}

int main(int argc, char* argv[]) {
	int nr_steps = 100;
	if (argc > 1)
		nr_steps = atoi(argv[1]);
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		grid_add_row(grid, line);
	}
	struct grid* grid2 = create_grid_with_size(grid->nr_rows, grid->nr_cols);
	for (int ii = 0; ii < nr_steps; ++ii) {
		step_grid(grid, grid2);
		// swap boards
		struct grid* t = grid;
		grid = grid2;
		grid2 = t;
		// print_grid(grid);
	}
	printf("%d\n", count_lights(grid));
	destroy_grid(grid2);
	destroy_grid(grid);
	free(line);
	return 0;
}
