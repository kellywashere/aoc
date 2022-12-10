#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

struct cell {
	int height;
	bool visible;
};

struct grid {
	int           w;
	int           h;
	int           rowcapacity; // max nr of rows reserved in mem
	struct cell*  grid;
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
		for (int ii = g->w * g->rowcapacity; ii < g->w * 2 * g->rowcapacity; ++ii) {
			g->grid[ii].height = 0;
			g->grid[ii].visible = false;
		}
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		g->grid[col + g->h * g->w].height = line[col] - '0';
		g->grid[col + g->h * g->w].visible = false;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x].height + '0';
			if (g->grid[y * g->w + x].visible)
				printf(GRN "%c" COL_RESET, c);
			else
				printf(RED "%c" COL_RESET, c);
		}
		printf("\n");
	}
}

void set_visible_dir(struct grid* g, int x, int y, int dx, int dy) {
	g->grid[y * g->w + x].visible = true;
	int maxh = g->grid[y * g->w + x].height;
	x += dx;
	y += dy;
	while (x >= 0 && x < g->w && y >= 0 && y < g->h) {
		int idx = y * g->w + x;
		if (g->grid[idx].height > maxh) {
			g->grid[idx].visible = true;
			maxh = g->grid[idx].height;
		}
		x += dx;
		y += dy;
	}
}

void set_visible(struct grid* g) {
	for (int y = 0; y < g->h; ++y)
		for (int x = 0; x < g->w; ++x)
			g->grid[y * g->w + x].visible = false;
	// horz
	for (int y = 0; y < g->h; ++y) {
		set_visible_dir(g,        0, y,  1, 0);
		set_visible_dir(g, g->w - 1, y, -1, 0);
	}
	// vert
	for (int x = 0; x < g->w; ++x) {
		set_visible_dir(g, x,        0,  0,  1);
		set_visible_dir(g, x, g->h - 1,  0, -1);
	}
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	set_visible(grid);
	show_grid(grid);

	int count = 0;
	for (int ii = 0; ii < grid->w * grid->h; ++ii)
		count += grid->grid[ii].visible ? 1 : 0;
	printf("%d\n", count);

	destroy_grid(grid);
	return 0;
}
