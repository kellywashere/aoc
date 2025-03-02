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
	int  height;
	bool reachable;
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
		int val = line[col] - '0';
		int idx = col + g->h * g->w;
		g->grid[idx].height = val;
		g->grid[idx].reachable = false;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			struct cell cell = g->grid[y * g->w + x];
			if (cell.reachable)
				printf(GRN "%c" COL_RESET, cell.height + '0');
			else
				printf("%c", cell.height + '0');
		}
		printf("\n");
	}
}

void grid_reset_reachable(struct grid* g) {
	for (int ii = 0; ii < g->w * g->h; ++ii)
		g->grid[ii].reachable = false;
}

bool grid_find_next_zero(struct grid* g, int* idx) {
	for (int ii = *idx; ii < g->w * g->h; ++ii) {
		if (g->grid[ii].height == 0) {
			*idx = ii;
			return true;
		}
	}
	return false;
}

const int dir2dx[] = { 0, 1, 0, -1 };
const int dir2dy[] = {-1, 0, 1,  0 };

void grid_mark_reachable(struct grid* g, int row, int col) {
	int idx = col + row * g->w;
	if (g->grid[idx].reachable) // already visited
		return;
	g->grid[idx].reachable = true;

	int height = g->grid[idx].height;
	if (height == 9)
		return;

	for (int dir = 0; dir < 4; ++dir) {
		int cn = col + dir2dx[dir];
		int rn = row + dir2dy[dir];
		if (cn >= 0 && cn < g->w && rn >= 0 && rn < g->h &&
			g->grid[cn + rn * g->w].height == height + 1) {
				grid_mark_reachable(g, rn, cn);
		}
	}
}

int grid_count_reachable_nines(struct grid* g) {
	int count = 0;
	for (int ii = 0; ii < g->w * g->h; ++ii)
		count += (g->grid[ii].reachable && g->grid[ii].height == 9) ? 1 : 0;
	return count;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	int idx0 = 0;
	int count = 0;
	while (grid_find_next_zero(grid, &idx0)) {
		int row0 = idx0 / grid->w;
		int col0 = idx0 % grid->w;

		grid_reset_reachable(grid);
		grid_mark_reachable(grid, row0, col0);
		count += grid_count_reachable_nines(grid);

		++idx0;
	}

	//show_grid(grid);

	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
