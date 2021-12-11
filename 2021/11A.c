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

struct dumbo {
	int  energy;
	bool has_flashed;
};

struct grid {
	size_t         nr_rows;
	size_t         nr_cols;
	struct dumbo** rows; // each row is its own piece of memory
	size_t         row_capacity; // max nr of rows
};

#define GRID_INIT_CAPACITY 2 /* deliberately too small, for test purp */

struct grid* create_grid() {
	struct grid* g = mymalloc(sizeof(struct grid));
	g->row_capacity = GRID_INIT_CAPACITY;
	g->rows = mymalloc(g->row_capacity * sizeof(struct dumbo*));
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
		g->rows = myrealloc(g->rows, g->row_capacity * sizeof(struct dumbo*));
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
	struct dumbo* newrow = mymalloc(nr_cols * sizeof(struct dumbo));
	for (int col = 0; col < nr_cols; ++col) {
		newrow[col].energy = rowstr[col] - '0';
		newrow[col].has_flashed = false;
	}
	g->rows[g->nr_rows++] = newrow;
}

// debug:
void print_grid(struct grid* g) {
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			printf("%d", g->rows[row][col].energy);
		}
		printf("\n");
	}
}

void grid_flash(struct grid* g, int row, int col) {
	int start_row = row > 0 ? row - 1 : 0;
	int end_row = row < g->nr_rows - 1 ? row + 1 : g->nr_rows - 1;
	int start_col = col > 0 ? col - 1 : 0;
	int end_col = col < g->nr_cols - 1 ? col + 1 : g->nr_cols - 1;
	for (int r = start_row; r <= end_row; ++r) {
		for (int c = start_col; c <= end_col; ++c) {
			++g->rows[r][c].energy;
		}
	}
	--g->rows[row][col].energy; // do not inc cell itself
	g->rows[row][col].has_flashed = true;
}

int grid_step(struct grid* g) {
	int flash_count = 0;
	// inc all cells
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			++g->rows[row][col].energy;
			g->rows[row][col].has_flashed = false;
		}
	}
	// flash loop
	bool flashed;
	do {
		flashed = false;
		for (int row = 0; row < g->nr_rows; ++row) {
			for (int col = 0; col < g->nr_cols; ++col) {
				if (g->rows[row][col].energy > 9 && !g->rows[row][col].has_flashed) {
					grid_flash(g, row, col);
					flashed = true;
					++flash_count;
				}
			}
		}
	} while (flashed);
	// put energies back in range
	for (int row = 0; row < g->nr_rows; ++row) {
		for (int col = 0; col < g->nr_cols; ++col) {
			if (g->rows[row][col].has_flashed)
				g->rows[row][col].energy = 0;
		}
	}
	return flash_count;
}

int main(int argc, char* argv[]) {
	int nr_iters = 100;
	if (argc > 1)
		nr_iters = atoi(argv[1]);
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		grid_add_row(grid, line);
	}
	int flash_count = 0;
	for (int ii = 0; ii < nr_iters; ++ii)
		flash_count += grid_step(grid);
	printf("Final grid:\n");
	print_grid(grid);
	printf("%d\n", flash_count);
	destroy_grid(grid);
	free(line);
	return 0;
}
