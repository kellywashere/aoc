#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <inttypes.h>

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

int count_trees(struct grid* g, int dx, int dy) {
	int x = 0;
	int trees = 0;
	for (int y = 0; y < g->gridh; y += dy) {
		trees += g->grid[x + y * g->gridw] == TREE ? 1 : 0;
		x = (x + dx) % g->gridw;
	}
	return trees;
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

	uint64_t prod = count_trees(grid, 1, 1);
	prod *= count_trees(grid, 3, 1);
	prod *= count_trees(grid, 5, 1);
	prod *= count_trees(grid, 7, 1);
	prod *= count_trees(grid, 1, 2);

	printf("%" PRIu64 "\n", prod);

	destroy_grid(grid);
	return 0;
}
