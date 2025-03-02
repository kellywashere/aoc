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
		int val = line[col] == '.' ? -1 : line[col] - '0';
		int idx = col + g->h * g->w;
		g->grid[idx].height = val;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			struct cell cell = g->grid[y * g->w + x];
			printf("%c", cell.height == -1 ? '.' : cell.height + '0');
		}
		printf("\n");
	}
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

int grid_count_paths(struct grid* g, int row, int col) {
	// recursivily counts paths toward a 9
	int count = 0;
	int idx = col + row * g->w;
	
	int height = g->grid[idx].height;
	if (height == 9)
		return 1;
	for (int dir = 0; dir < 4; ++dir) {
		int cn = col + dir2dx[dir];
		int rn = row + dir2dy[dir];
		if (cn >= 0 && cn < g->w && rn >= 0 && rn < g->h &&
			g->grid[cn + rn * g->w].height == height + 1) {
				count += grid_count_paths(g, rn, cn);
		}
	}
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

		count += grid_count_paths(grid, row0, col0);

		++idx0;
	}

	//show_grid(grid);

	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
