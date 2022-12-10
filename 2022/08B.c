#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

struct cell {
	int height;
	int viewdist[4]; // east, north, west, south
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
		}
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		g->grid[col + g->h * g->w].height = line[col] - '0';
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x].height + '0';
			printf("%c", c);
		}
		printf("\n");
	}
}

int dir2dx[] = { 1, 0, -1, 0};
int dir2dy[] = { 0, -1, 0, 1};

void set_viewdist_dir(struct grid* g, int dir, int x, int y) {
	int lasth[10];
	for (int h = 0; h <= 9; ++h)
		lasth[h] = -1;
	int d = 0; // distance from edge
	while (x >= 0 && x < g->w && y >= 0 && y < g->h) {
		int idx = y * g->w + x;
		int h = g->grid[idx].height;
		g->grid[idx].viewdist[dir] = d;
		for (int hh = h; hh <= 9; ++hh) {
			if (lasth[hh] >= 0 && (d - lasth[hh]) < g->grid[idx].viewdist[dir])
				g->grid[idx].viewdist[dir] = (d - lasth[hh]);
		}
		lasth[h] = d;
		x += dir2dx[dir];
		y += dir2dy[dir];
		++d;
	}
}

void set_viewdist(struct grid* g) {
	// horz
	for (int y = 0; y < g->h; ++y) {
		set_viewdist_dir(g, 0,        0, y);
		set_viewdist_dir(g, 2, g->w - 1, y);
	}
	// vert
	for (int x = 0; x < g->w; ++x) {
		set_viewdist_dir(g, 1, x, g->h - 1);
		set_viewdist_dir(g, 3, x,        0);
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

	set_viewdist(grid);
	//show_grid(grid);

	int maxprod = 0;
	for (int ii = 0; ii < grid->w * grid->h; ++ii) {
		int prod = 1;
		for (int dir = 0; dir < 4; ++dir)
			prod *= grid->grid[ii].viewdist[dir];
		maxprod = prod > maxprod ? prod : maxprod;
	}
	printf("%d\n", maxprod);

	destroy_grid(grid);
	return 0;
}
