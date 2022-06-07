#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define GRIDSIZE 201

struct grid {
	int   gridw;
	int   gridh;
	int   rowcapacity; // max nr of rows reserved in mem
	bool* grid;
	// position and direction of carrier
	int   posx;
	int   posy;
	int   dirx;
	int   diry;
	// infection count
	int   infection_count;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	g->posx = 0;
	g->posy = 0;
	g->dirx = 0;
	g->diry = -1;
	g->infection_count = 0;
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
	while (!isspace(line[lw]))
		++lw;
	if (g->gridw > 0 && g->gridw != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return;
	}
	if (!g->grid) {
		g->gridw = lw;
		g->gridh = 0;
		g->rowcapacity = lw; // assume square
		g->grid = calloc(g->gridw * g->rowcapacity, sizeof(bool));
	}
	if (g->gridh >= g->rowcapacity) {
		g->grid = realloc(g->grid, g->gridw * 2 * g->rowcapacity * sizeof(bool));
		for (int ii = g->gridw * g->rowcapacity; ii < g->gridw * 2 * g->rowcapacity; ++ii)
			g->grid[ii] = false;
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->gridw; ++col)
		g->grid[col + g->gridh * g->gridw] = line[col] == '#';
	++g->gridh;
	g->posx = (g->gridw - 1) / 2;
	g->posy = (g->gridh - 1) / 2;
}

struct grid* enlarge_grid(struct grid* g, int newsize) {
	if (newsize < g->gridw || newsize < g->gridh) {
		fprintf(stderr, "Grid w/ size %dx%d does not fit new size %dx%d\n", g->gridw, g->gridh, newsize, newsize);
		return g;
	}
	struct grid* ng = create_grid();
	ng->gridw = newsize;
	ng->gridh = newsize;
	ng->rowcapacity = newsize;
	ng->grid = calloc(newsize * newsize, sizeof(bool));
	// copy g into, keeping centers aligned
	int cols_left = (newsize - g->gridw) / 2;
	int rows_top = (newsize - g->gridh) / 2;
	ng->posx = g->posx + cols_left;
	ng->posy = g->posy + rows_top;
	for (int y = 0; y < g->gridh; ++y)
		for (int x = 0; x < g->gridw; ++x)
			ng->grid[(y + rows_top) * newsize + (x + cols_left)] = g->grid[y * g->gridw + x];
	ng->dirx = g->dirx;
	ng->diry = g->diry;
	ng->infection_count = g->infection_count;
	destroy_grid(g);
	return ng;
}


void step(struct grid** pG) {
	struct grid* g = *pG;
	bool v = g->grid[g->posy * g->gridw + g->posx];
	g->grid[g->posy * g->gridw + g->posx] = !v;
	g->infection_count += !v ? 1 : 0;
	int rot = v ? 1 : -1; // -1: left, 1: right
	int dx_next = -rot * g->diry;
	g->diry = rot * g->dirx;
	g->dirx = dx_next;
	g->posx += g->dirx;
	g->posy += g->diry;
	if (g->posx < 0 || g->posx >= g->gridw || g->posy < 0 || g->posy >= g->gridh) {
		g = enlarge_grid(g, 2*g->gridw - 1);
		*pG = g;
	}
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			char c = g->grid[y * g->gridw + x] ? '#' : '.';
			if (g->posx == x && g->posy == y)
				printf("[%c]", c);
			else
				printf(" %c ", c);
		}
		printf("\n");
	}
	printf("%c\n", g->dirx == 1 ? '>' : g->dirx == -1 ? '<' : g->diry == -1 ? '^' : 'v');
}

int main(int argc, char* argv[]) {
	// grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(bool));
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	// show_grid(grid);
	for (int ii = 0; ii < 10000; ++ii) {
		step(&grid);
		// printf("\n");
		// show_grid(grid);
	}
	printf("%d\n", grid->infection_count);
	
	// free(grid);
	destroy_grid(grid);
	return 0;
}
