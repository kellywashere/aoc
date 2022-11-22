#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

enum cell_type {
	EMPTY,
	TREE
};

struct grid {
	int             gridw;
	int             gridh;
	int             rowcapacity; // max nr of rows reserved in mem
	enum cell_type* grid;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
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

bool grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (!isspace(line[lw]))
		++lw;
	if (g->gridw > 0 && g->gridw != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return false;
	}
	if (!g->grid) {
		g->gridw = lw;
		g->gridh = 0;
		g->rowcapacity = lw; // assume square
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(enum cell_type));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(enum cell_type));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx] = line[col] == '#' ? TREE : EMPTY;
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			char c = g->grid[idx] == TREE ? '#' : '.';
			printf("%c", c);
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!grid_add_line(grid, line))
			break;
	}
	free(line);

	int x = 0;
	int trees = 0;
	for (int y = 0; y < grid->gridh; ++y) {
		trees += grid->grid[x + y * grid->gridw] == TREE ? 1 : 0;
		x = (x + 3) % grid->gridw;
	}
	printf("%d\n", trees);

	destroy_grid(grid);
	return 0;
}
