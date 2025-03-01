#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

struct cell {
	bool antenna;
	char val;
	bool is_antinode;
};

struct grid {
	int            w;
	int            h;
	int            rowcapacity; // max nr of rows reserved in mem
	struct cell*   grid;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = 0;
	g->h = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (line[lw] && !isspace(line[lw]))
		++lw;
	if (g->w > 0 && g->w != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return;
	}
	if (!g->grid) {
		g->w = lw;
		g->h = 0;
		g->rowcapacity = lw; // assume square
		g->grid = calloc(g->w * g->rowcapacity, sizeof(struct cell));
	}
	if (g->h >= g->rowcapacity) {
		g->grid = realloc(g->grid, g->w * 2 * g->rowcapacity * sizeof(struct cell));
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		char c = line[col];
		int idx = col + g->h * g->w;
		g->grid[idx].antenna = c != '.';
		g->grid[idx].val = c;
		g->grid[idx].is_antinode = false;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			struct cell cell = g->grid[y * g->w + x];
			char c = cell.antenna ? cell.val : cell.is_antinode ? '#' : '.';
			if (cell.is_antinode)
				printf(GRN "%c" COL_RESET, c);
			else if (cell.antenna)
				printf(RED "%c" COL_RESET, c);
			else
				printf("%c", c);
		}
		printf("\n");
	}
}

bool find_antenna(struct grid* g, char val, int startidx, int* foundidx) {
	for (int ii = startidx; ii < g->w * g->h; ++ii) {
		if (g->grid[ii].antenna && g->grid[ii].val == val) {
			*foundidx = ii;
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	int idx1 = 0;
	for (int row1 = 0; row1 < grid->h; ++row1) {
		for (int col1 = 0; col1 < grid->w; ++col1) {
			struct cell cell1 = grid->grid[idx1];
			if (cell1.antenna) {
				int idx2 = idx1 + 1;
				while (find_antenna(grid, cell1.val, idx2, &idx2)) {
					int row2 = idx2 / grid->w;
					int col2 = idx2 % grid->w;
					int r = 2 * row1 - row2;
					int c = 2 * col1 - col2;
					if (r >= 0 && r < grid->h && c >= 0 && c < grid->w) {
						grid->grid[c + grid->w * r].is_antinode = true;
					}
					r = 2 * row2 - row1;
					c = 2 * col2 - col1;
					if (r >= 0 && r < grid->h && c >= 0 && c < grid->w) {
						grid->grid[c + grid->w * r].is_antinode = true;
					}
					++idx2;
				}
			}
			++idx1;
		}
	}

	//show_grid(grid);

	int count = 0;
	for (int ii = 0; ii < grid->w * grid->h; ++ii)
		count += grid->grid[ii].is_antinode ? 1 : 0;

	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
