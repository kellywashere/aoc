#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

enum cell_type {
	FLOOR,
	EMPTY,
	OCCUPIED
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
		g->grid[idx] = line[col] == 'L' ? EMPTY : line[col] == '#' ? OCCUPIED : FLOOR;
	}
	++g->gridh;
	return true;
}

void grid_copy_fromto(struct grid* from, struct grid* to) {
	memcpy(to->grid, from->grid, from->gridw * from->gridh * sizeof(enum cell_type));
}

struct grid* grid_copy(struct grid* g) {
	struct grid* newg = malloc(sizeof(struct grid));
	newg->gridw = g->gridw;
	newg->gridh = g->gridh;
	newg->rowcapacity = g->gridh;
	newg->grid = malloc(newg->gridw * newg->rowcapacity * sizeof(enum cell_type));
	grid_copy_fromto(g, newg);
	return newg;
}

int dir2dx[] = {1,  1,  0, -1, -1, -1, 0, 1};
int dir2dy[] = {0, -1, -1, -1,  0,  1, 1, 1};

int count_occupied(struct grid* g, int x, int y) {
	// count occupied visible seats including this one
	int count = g->grid[y * g->gridw + x] == OCCUPIED ? 1 : 0;
	for (int dir = 0; dir < 8; ++dir) {
		int dx = dir2dx[dir];
		int dy = dir2dy[dir];
		bool occ = false;
		int xx = x + dx;
		int yy = y + dy;
		bool done = false;
		while (!done && xx >= 0 && xx < g->gridw && yy >= 0 && yy < g->gridh) {
			enum cell_type c = g->grid[yy * g->gridw + xx];
			if (c == OCCUPIED)
				occ = true;
			if (c != FLOOR)
				done = true;
			xx += dx;
			yy += dy;
		}
		count += occ ? 1 : 0;
	}
	return count;
}

bool one_step(struct grid* in, struct grid* out) {
	// does one algo step
	// returns true if change was made
	bool changed = false;
	for (int y = 0; y < in->gridh; ++y) {
		for (int x = 0; x < in->gridw; ++x) {
			int idx = y * in->gridw + x;
			out->grid[idx] = in->grid[idx]; // unless changed later
			if (in->grid[idx] != FLOOR) {
				int occ = count_occupied(in, x, y);
				if (occ == 0) {
					out->grid[idx] = OCCUPIED;
					changed = true;
				}
				else if (in->grid[idx] == OCCUPIED && occ >= 6) { // 6 inc this chair
					out->grid[idx] = EMPTY;
					changed = true;
				}
			}
		}
	}
	return changed;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			char c = g->grid[idx] == FLOOR ? '.' : g->grid[idx] == EMPTY ? 'L' : '#';
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

	struct grid* next = grid_copy(grid);

	bool changed;
	do {
		changed = one_step(grid, next);
		struct grid* tmp = next;
		next = grid;
		grid = tmp;
	} while (changed);
	// show_grid(grid);
	int count = 0;
	for (int idx = 0; idx < grid->gridw * grid->gridh; ++idx)
		count += grid->grid[idx] == OCCUPIED ? 1 : 0;
	printf("%d\n", count);

	destroy_grid(grid);
	destroy_grid(next);
	return 0;
}
