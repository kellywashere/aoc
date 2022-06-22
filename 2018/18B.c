#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h> /* usleep */

enum cell {
	OPEN,
	TREE,
	LUMBERYARD
};

struct grid {
	int        gridw;
	int        gridh;
	int        rowcapacity; // max nr of rows reserved in mem
	enum cell* grid;
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
	// does not destroy entities in grid
	if (g) {
		free(g->grid);
		free(g);
	}
}

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
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(enum cell));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(enum cell));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx] = line[col] == '|' ? TREE : line[col] == '#' ? LUMBERYARD : OPEN;
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			enum cell c = g->grid[y * g->gridw + x];
			printf("%c", c == OPEN ? '.' : c == TREE ? '|' : '#');
		}
		printf("\n");
	}
}

void single_step(struct grid* from, struct grid* to) {
	int neighbor_count[3];
	for (int y = 0; y < from->gridh; ++y) {
		for (int x = 0; x < from->gridw; ++x) {
			int idx = y * from->gridw + x;
			// count 8 neighbors
			for (int ii = 0; ii < 3; ++ii)
				neighbor_count[ii] = 0;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int yy = y + dy;
					int xx = x + dx;
					if ((dx != 0 || dy != 0) && yy >= 0 && yy < from->gridh && xx >= 0 && xx < from->gridw)
						++neighbor_count[from->grid[yy * from->gridw + xx]];
				}
			}
			// update to cell
			if (from->grid[idx] == OPEN)
				to->grid[idx] = neighbor_count[TREE] >= 3 ? TREE : OPEN;
			else if (from->grid[idx] == TREE)
				to->grid[idx] = neighbor_count[LUMBERYARD] >= 3 ? LUMBERYARD : TREE;
			else // LUMBERYARD
				to->grid[idx] = (neighbor_count[LUMBERYARD] >= 1 && neighbor_count[TREE] >= 1) ? LUMBERYARD : OPEN;
		}
	}
}

bool grids_equal(struct grid* g1, struct grid* g2) {
	if (g1->gridw != g2->gridw || g1->gridh != g2->gridh)
		return false;
	int s = g1->gridw * g1->gridh;
	for (int ii = 0; ii < s; ++ii)
		if (g1->grid[ii] != g2->grid[ii])
			return false;
	return true;
}

void floyd(struct grid* g, int* mu, int* lambda) {
	// tortoise-hare cycle detection
	// https://en.wikipedia.org/wiki/Cycle_detection
	// ** NOTE NOTE NOTE **
	// For speed of of puzzle solution: leaves g in state after mu steps!!!
	struct grid* tortoise = grid_copy(g);
	struct grid* hare = grid_copy(g);
	struct grid* new = grid_copy(g);
	single_step(g, tortoise);  // f(x0)
	single_step(tortoise, hare); // f(f(x0))
	while (!grids_equal(tortoise, hare)) {
		// tortoise := f(tortoise)
		single_step(tortoise, new);
		// swap new <-> tortoise
		struct grid* tmp = tortoise;
		tortoise = new;
		new = tmp;

		// hare = f(f(hare))
		single_step(hare, new);
		single_step(new, hare);
	}
	// find mu
	*mu = 0;
	grid_copy_fromto(g, tortoise); // tortoise = x0
	while (!grids_equal(tortoise, hare)) {
		// tortoise := f(tortoise)
		single_step(tortoise, new);
		// swap new <-> tortoise
		struct grid* tmp = tortoise;
		tortoise = new;
		new = tmp;

		// hare := f(hare)
		single_step(hare, new);
		// swap new <-> hare
		tmp = hare;
		hare = new;
		new = tmp;

		++(*mu);
	}
	// leave g in state after mu steps!
	grid_copy_fromto(tortoise, g);

	// find lambda
	*lambda = 1;
	// hare = f(tortoise)
	single_step(tortoise, hare);
	while (!grids_equal(tortoise, hare)) {
		// hare := f(hare)
		single_step(hare, new);
		// swap new <-> hare
		struct grid* tmp = hare;
		hare = new;
		new = tmp;

		++(*lambda);
	}

	destroy_grid(tortoise);
	destroy_grid(hare);
	destroy_grid(new);
}

int main(int argc, char* argv[]) {
	int steps = 1000000000;

	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		grid_add_line(grid, line);
	free(line);

	int mu, lambda;
	floyd(grid, &mu, &lambda); // For speed of of puzzle solution: leaves g in state after mu steps!!!
	// printf("mu = %d, lambda = %d\n", mu, lambda);

	steps -= mu; // we already did mu steps in floyd()
	steps %= lambda; // skip cycles

	struct grid* newgrid = grid_copy(grid);
	for (int step = 0; step < steps; ++step) {
		single_step(grid, newgrid);
		struct grid* tmp = grid;
		grid = newgrid;
		newgrid = tmp;
	}

	// puzzle answer: #tree * #lumberyard
	int trees = 0;
	int lumberyards = 0;
	for (int ii = 0; ii < grid->gridw * grid->gridh; ++ii) {
		trees       += grid->grid[ii] == TREE       ? 1 : 0;
		lumberyards += grid->grid[ii] == LUMBERYARD ? 1 : 0;
	}
	printf("%d\n", trees * lumberyards);

	destroy_grid(newgrid);
	destroy_grid(grid);
	return 0;
}
