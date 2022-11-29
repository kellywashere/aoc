#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)<0?(-(x)):(x))

#define NR_STEPS 6

// The cube will always be symmetric around original 2d grid

struct grid2d {
	int   gridw;
	int   gridh;
	int   rowcapacity; // max nr of rows reserved in mem
	bool* grid;
};

struct grid3d {
	int   size_x; // 0 <= x < size_x
	int   size_y;
	int   size_z;
	bool* grid;
};

struct grid2d* create_grid2d() {
	struct grid2d* g = malloc(sizeof(struct grid2d));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	return g;
}

void destroy_grid2d(struct grid2d* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool grid2d_add_line(struct grid2d* g, char* line) {
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
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(bool));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(bool));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx] = line[col] == '#';
	}
	++g->gridh;
	return true;
}

void show_grid2d(struct grid2d* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			char c = g->grid[idx] ? '#' : '.';
			printf("%c", c);
		}
		printf("\n");
	}
}

struct grid3d* create_grid3d(int size_x, int size_y, int size_z) {
	struct grid3d* g3d = malloc(sizeof(struct grid3d));
	g3d->size_x = size_x;
	g3d->size_y = size_y;
	g3d->size_z = size_z;
	g3d->grid = calloc(size_x * size_y * size_z, sizeof(bool));
	return g3d;
}

void grid3d_set(struct grid3d* g, int x, int y, int z, bool val) {
	int idx = z * g->size_y * g->size_x + y * g->size_x + x;
	g->grid[idx] = val;
}

bool grid3d_get(struct grid3d* g, int x, int y, int z) {
	int idx = z * g->size_y * g->size_x + y * g->size_x + x;
	return g->grid[idx];
}

struct grid3d* create_grid3d_from_grid2d(int size_x, int size_y, int size_z, struct grid2d* g2d) {
	struct grid3d* g3d = create_grid3d(size_x, size_y, size_z);
	// copy g2d to z=0 plane
	int xoffs = (size_x - g2d->gridw)/2;
	int yoffs = (size_y - g2d->gridh)/2;
	for (int x = 0; x < g2d->gridw; ++x) {
		for (int y = 0; y < g2d->gridh; ++y) {
			bool v = g2d->grid[y * g2d->gridw + x];
			grid3d_set(g3d, x + xoffs, y + yoffs, 0, v);
		}
	}
	return g3d;
}

void destroy_grid3d(struct grid3d* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void show_grid3d(struct grid3d* g) {
	for (int z = 0; z < g->size_z; ++z) {
		printf("z = %d\n", z);
		for (int y = 0; y < g->size_y; ++y) {
			for (int x = 0; x < g->size_x; ++x) {
				printf("%c", grid3d_get(g, x,y,z) ? '#' : '.');
			}
			printf("\n");
		}
	}
}

int count_active_neighbors(struct grid3d* g, int x, int y, int z) {
	int count = grid3d_get(g, x, y, z) ? -1 : 0; // do not count this one
	for (int dz = -1; dz <= 1; ++dz) {
		int zz = ABS(z + dz);
		for (int dy = -1; dy <= 1; ++dy) {
			int yy = y + dy;
			for (int dx = -1; dx <= 1; ++dx) {
				int xx = x + dx;
				if (zz < g->size_z && yy >= 0 && yy < g->size_y && xx >= 0 && xx < g->size_x)
					count += grid3d_get(g, xx, yy, zz) ? 1 : 0;
			}
		}
	}
	return count;
}

void one_step(struct grid3d* in, struct grid3d* out) {
	for (int z = 0; z < in->size_z; ++z) {
		for (int y = 0; y < in->size_y; ++y) {
			for (int x = 0; x < in->size_x; ++x) {
				int neighbors = count_active_neighbors(in, x, y, z);
				//printf("Nr neighbors of (%d,%d,%d): %d\n", x,y,z, neighbors);
				bool active = grid3d_get(in, x, y, z);
				bool newval = (active && neighbors == 2) || neighbors == 3;
				grid3d_set(out, x, y, z, newval);
			}
		}
	}
}

int grid3d_count_active_cells(struct grid3d* g) {
	int count = 0;
	for (int z = 0; z < g->size_z; ++z) {
		for (int y = 0; y < g->size_y; ++y) {
			for (int x = 0; x < g->size_x; ++x) {
				count += grid3d_get(g, x, y, z) ? (z == 0 ? 1 : 2) : 0;
			}
		}
	}
	return count;
}

int main(int argc, char* argv[]) {
	struct grid2d* grid2d = create_grid2d();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!grid2d_add_line(grid2d, line))
			break;
	}
	free(line);

	// show_grid2d(grid2d);
	int size_x = grid2d->gridw + 2 * NR_STEPS;
	int size_y = grid2d->gridh + 2 * NR_STEPS;
	int size_z = 1 + NR_STEPS; // z-symmetry
	struct grid3d* grid3d = create_grid3d_from_grid2d(size_x, size_y, size_z, grid2d);
	destroy_grid2d(grid2d);

	// show_grid3d(grid3d);
	
	struct grid3d* nextgrid = create_grid3d(size_x, size_y, size_z);

	for (int ii = 0; ii < NR_STEPS; ++ii) {
		one_step(grid3d, nextgrid);
		struct grid3d* tmp = grid3d;
		grid3d = nextgrid;
		nextgrid = tmp;
	}
	// count active cells
	printf("%d\n", grid3d_count_active_cells(grid3d));

	destroy_grid3d(grid3d);
	destroy_grid3d(nextgrid);
	return 0;
}
