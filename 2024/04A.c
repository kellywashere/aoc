#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h> /* usleep */

struct grid {
	int          w;
	int          h;
	int          rowcapacity; // max nr of rows reserved in mem
	char*        grid;
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
	// does not destroy entities in grid
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (!isspace(line[lw]))
		++lw;
	if (g->w > 0 && g->w != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return false;
	}
	if (!g->grid) {
		g->w = lw;
		g->h = 0;
		g->rowcapacity = lw; // assume square
		g->grid = malloc(g->w * g->rowcapacity * sizeof(char));
	}
	if (g->h >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->w * g->rowcapacity * sizeof(char));
	}
	for (int col = 0; col < g->w; ++col) {
		int idx = col + g->h * g->w;
		g->grid[idx] = line[col];
	}
	++g->h;
	return true;
}

char get_cell(struct grid* g, int row, int col) {
	int idx = row * g->w + col;
	return g->grid[idx];
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			int idx = y * g->w + x;
			printf("%c", g->grid[idx]);
		}
		printf("\n");
	}
}

int dir2dx[] = { 0,  1, 1, 1, 0, -1, -1, -1};
int dir2dy[] = {-1, -1, 0, 1, 1,  1,  0, -1};
const int nrdirs = 8;

bool is_xmas(struct grid* g, int row, int col, int drow, int dcol) {
	char* xmas = "XMAS";
	for (int ii = 0; ii < 4; ++ii) {
		if (row < 0 || row >= g->h || col < 0 || col >= g->w)
			return false;
		char cell = get_cell(g, row, col);
		if (cell != xmas[ii])
			return false;
		row += drow;
		col += dcol;
	}
	return true;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		grid_add_line(grid, line);
	free(line);

	int count = 0;
	for (int row = 0; row < grid->h; ++row) {
		for (int col = 0; col < grid->w; ++col) {
			for (int dir = 0; dir < nrdirs; ++dir) {
				count += is_xmas(grid, row, col, dir2dy[dir], dir2dx[dir]) ? 1 : 0;
			}
		}
	}
	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
