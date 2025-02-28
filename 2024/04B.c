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

bool is_xmas(struct grid* g, int row, int col) {
	if (get_cell(g, row, col) != 'A')
		return false;
	char ul = get_cell(g, row - 1, col - 1);
	char ur = get_cell(g, row - 1, col + 1);
	char bl = get_cell(g, row + 1, col - 1);
	char br = get_cell(g, row + 1, col + 1);
	if ( !((ul == 'M' && br == 'S') || (ul == 'S' && br == 'M')))
		return false;
	if ( !((ur == 'M' && bl == 'S') || (ur == 'S' && bl == 'M')))
		return false;
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
	for (int row = 1; row < grid->h - 1; ++row) {
		for (int col = 1; col < grid->w - 1; ++col) {
			count += is_xmas(grid, row, col) ? 1 : 0;
		}
	}
	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
