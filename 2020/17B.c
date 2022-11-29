#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)<0?(-(x)):(x))

#define NR_STEPS 6

// The hyper-cube will always be symmetric around original 2d grid around z=0 and w=0

struct grid2d {
	int   gridw;
	int   gridh;
	int   rowcapacity; // max nr of rows reserved in mem
	bool* grid;
};

struct grid4d {
	int   size_x; // 0 <= x < size_x
	int   size_y;
	int   size_z;
	int   size_w;
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

struct grid4d* create_grid4d(int size_x, int size_y, int size_z, int size_w) {
	struct grid4d* g4d = malloc(sizeof(struct grid4d));
	g4d->size_x = size_x;
	g4d->size_y = size_y;
	g4d->size_z = size_z;
	g4d->size_w = size_w;
	g4d->grid = calloc(size_x * size_y * size_z * size_w, sizeof(bool));
	return g4d;
}

void grid4d_set(struct grid4d* g, int x, int y, int z, int w, bool val) {
	int idx = w * g->size_z * g->size_y * g->size_x + z * g->size_y * g->size_x + y * g->size_x + x;
	g->grid[idx] = val;
}

bool grid4d_get(struct grid4d* g, int x, int y, int z, int w) {
	int idx = w * g->size_z * g->size_y * g->size_x + z * g->size_y * g->size_x + y * g->size_x + x;
	return g->grid[idx];
}

struct grid4d* create_grid4d_from_grid2d(int size_x, int size_y, int size_z, int size_w, struct grid2d* g2d) {
	struct grid4d* g4d = create_grid4d(size_x, size_y, size_z, size_w);
	// copy g2d to z=0, w=0 plane
	int xoffs = (size_x - g2d->gridw)/2;
	int yoffs = (size_y - g2d->gridh)/2;
	for (int x = 0; x < g2d->gridw; ++x) {
		for (int y = 0; y < g2d->gridh; ++y) {
			bool v = g2d->grid[y * g2d->gridw + x];
			grid4d_set(g4d, x + xoffs, y + yoffs, 0, 0, v);
		}
	}
	return g4d;
}

void destroy_grid4d(struct grid4d* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void show_grid4d(struct grid4d* g) {
	for (int w = 0; w < g->size_w; ++w) {
		for (int z = 0; z < g->size_z; ++z) {
			printf("z = %d, w = %d\n", z, w);
			for (int y = 0; y < g->size_y; ++y) {
				for (int x = 0; x < g->size_x; ++x)
					printf("%c", grid4d_get(g, x,y,z,w) ? '#' : '.');
				printf("\n");
			}
		}
	}
}

int count_active_neighbors(struct grid4d* g, int x, int y, int z, int w) {
	int count = grid4d_get(g, x, y, z, w) ? -1 : 0; // do not count this one
	for (int dw = -1; dw <= 1; ++dw) {
		int ww = ABS(w + dw);
		for (int dz = -1; dz <= 1; ++dz) {
			int zz = ABS(z + dz);
			for (int dy = -1; dy <= 1; ++dy) {
				int yy = y + dy;
				for (int dx = -1; dx <= 1; ++dx) {
					int xx = x + dx;
					if (ww < g->size_w && zz < g->size_z && yy >= 0 && yy < g->size_y && xx >= 0 && xx < g->size_x)
						count += grid4d_get(g, xx, yy, zz, ww) ? 1 : 0;
				}
			}
		}
	}
	return count;
}

void one_step(struct grid4d* in, struct grid4d* out) {
	for (int w = 0; w < in->size_w; ++w) {
		for (int z = 0; z < in->size_z; ++z) {
			for (int y = 0; y < in->size_y; ++y) {
				for (int x = 0; x < in->size_x; ++x) {
					int neighbors = count_active_neighbors(in, x, y, z, w);
					//printf("Nr neighbors of (%d,%d,%d,%d): %d\n", x,y,z,w, neighbors);
					bool active = grid4d_get(in, x, y, z, w);
					bool newval = (active && neighbors == 2) || neighbors == 3;
					grid4d_set(out, x, y, z, w, newval);
				}
			}
		}
	}
}

int grid4d_count_active_cells(struct grid4d* g) {
	int count = 0;
	for (int w = 0; w < g->size_w; ++w) {
		for (int z = 0; z < g->size_z; ++z) {
			for (int y = 0; y < g->size_y; ++y) {
				for (int x = 0; x < g->size_x; ++x) {
					int c = grid4d_get(g, x, y, z, w) ? 1 : 0;
					c *= z == 0 ? 1 : 2;
					c *= w == 0 ? 1 : 2;
					count += c;
				}
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
	int size_w = 1 + NR_STEPS; // w-symmetry
	struct grid4d* grid4d = create_grid4d_from_grid2d(size_x, size_y, size_z, size_w, grid2d);
	destroy_grid2d(grid2d);

	// show_grid4d(grid4d);
	
	struct grid4d* nextgrid = create_grid4d(size_x, size_y, size_z, size_w);

	for (int ii = 0; ii < NR_STEPS; ++ii) {
		one_step(grid4d, nextgrid);
		struct grid4d* tmp = grid4d;
		grid4d = nextgrid;
		nextgrid = tmp;
	}
	// count active cells
	printf("%d\n", grid4d_count_active_cells(grid4d));

	destroy_grid4d(grid4d);
	destroy_grid4d(nextgrid);
	return 0;
}
