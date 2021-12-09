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

struct coord {
	int row;
	int col;
	struct coord* next; // to allow linked-list / stack
};

struct coord* create_coord(int row, int col) {
	struct coord* c = mymalloc(sizeof(struct coord));
	c->row = row;
	c->col = col;
	c->next = NULL;
	return c;
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

// Stack ops

void push(struct coord** stack, int row, int col) {
	struct coord* c = create_coord(row, col);
	c->next = *stack;
	*stack = c;
}

void pop(struct coord** stack, int* row, int* col) {
	struct coord* c = *stack;
	*stack = c->next;
	*row = c->row;
	*col = c->col;
	free(c);
}

int flood_fill(struct grid* g, int row, int col) {
	// returns size of filled region
	// destructive: sets height to 9 when filled; fine for this problem
	int count = 0;
	if (g->rows[row][col] == 9) // height 9, or already filled
		return 0;
	struct coord* stack = NULL;
	push(&stack, row, col);
	while (stack) {
		pop(&stack, &row, &col);
		if (g->rows[row][col] == 9)
			continue;
		g->rows[row][col] = 9;
		++count;
		// check neighbors
		if (row > 0 && g->rows[row - 1][col] != 9)
			push(&stack, row - 1, col);
		if (row < g->nr_rows - 1 && g->rows[row + 1][col] != 9)
			push(&stack, row + 1, col);
		if (col > 0 && g->rows[row][col - 1] != 9)
			push(&stack, row, col - 1);
		if (col < g->nr_cols - 1 && g->rows[row][col + 1] != 9)
			push(&stack, row, col + 1);
	}
	return count;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
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
	int max_regions[3] = {0};
	for (int row = 0; row < grid->nr_rows; ++row) {
		for (int col = 0; col < grid->nr_cols; ++col) {
			int size = flood_fill(grid, row, col);
			if (size > max_regions[0]) {
				max_regions[0] = size;
				qsort(max_regions, 3, sizeof(int), cmp);
			}
		}
	}
	printf("%d\n", max_regions[0] * max_regions[1] * max_regions[2]);
	destroy_grid(grid);
	free(line);
	return 0;
}
