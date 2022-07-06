#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h> /* usleep */

struct cell {
	bool asteroid;
	// int  dist_sq; // distance squared
	bool visible;
};

struct grid {
	int          gridw;
	int          gridh;
	int          rowcapacity; // max nr of rows reserved in mem
	struct cell* grid;
};

int gcd(int a, int b) {
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;
	while (a != 0 && b != 0) {
		int na = b;
		b = a % b;
		a = na;
	}
	return b == 0 ? a : b;
}

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
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

/*
void grid_copy_fromto(struct grid* from, struct grid* to) {
	memcpy(to->grid, from->grid, from->gridw * from->gridh * sizeof(enum cell));
}

struct grid* grid_copy(struct grid* g) {
	struct grid* newg = malloc(sizeof(struct grid));
	newg->gridw = g->gridw;
	newg->gridh = g->gridh;
	newg->rowcapacity = g->gridh;
	newg->grid = malloc(newg->gridw * newg->rowcapacity * sizeof(enum cell));
	grid_copy_fromto(g, newg);
	return newg;
}
*/

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
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(struct cell));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(struct cell));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx].asteroid = line[col] == '#';
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			printf("%c", g->grid[idx].asteroid ? (g->grid[idx].visible ? '#' : '*') : '.');
		}
		printf("\n");
	}
}

int count_visible(struct grid* g, int x0, int y0) {
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].visible = g->grid[ii].asteroid;
	g->grid[y0 * g->gridw + x0].visible = false;
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			if (g->grid[y * g->gridw + x].visible) {
				int dx = x - x0;
				int dy = y - y0;
				int gcdiv = gcd(dx, dy);
				dx /= gcdiv; // simplify dy/dx to make smallest steps in this dir as possible
				dy /= gcdiv;
				int x1 = x0 + dx;
				int y1 = y0 + dy;
				int seen = 0;
				while (0 <= x1 && x1 < g->gridw && 0 <= y1 && y1 < g->gridh) {
					int idx = y1 * g->gridw + x1;
					if (g->grid[idx].asteroid) {
						g->grid[idx].visible = seen == 0;
						++seen;
					}
					x1 += dx;
					y1 += dy;
				}
			}
		}
	}
	// count
	int count = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		count += g->grid[ii].visible ? 1 : 0;
	return count;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		grid_add_line(grid, line);
	free(line);

	int maxvis = 0;
	for (int y = 0; y < grid->gridh; ++y) {
		for (int x = 0; x < grid->gridw; ++x) {
			if (grid->grid[y * grid->gridw + x].asteroid) {
				int vis = count_visible(grid, x, y);
				maxvis = vis > maxvis ? vis : maxvis;
			}
		}
	}
	printf("%d\n", maxvis);

	destroy_grid(grid);
	return 0;
}
