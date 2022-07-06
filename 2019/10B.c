#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h> /* usleep */

struct cell {
	bool asteroid;
	bool visible;
};

struct vec {
	int x;
	int y;
	bool exists; // when shot, exists becomes false
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
			printf("%c", g->grid[idx].asteroid ? '#' : '.');
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

void find_coordinates(struct grid* g, int* pX, int* pY) {
	int maxvis = 0;
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			if (g->grid[y * g->gridw + x].asteroid) {
				int vis = count_visible(g, x, y);
				if (vis > maxvis) {
					maxvis = vis;
					*pX = x;
					*pY = y;
				}
			}
		}
	}
}

int cross_prod(struct vec* va, struct vec* vb) {
	return va->x * vb->y - vb->x * va->y;
}

int cmp_vec(const void* a, const void* b) {
	// sorting based on how far the (0,-1) vector has to be rotated conterclkwise
	// to be aligned with va, vb
	struct vec* va = (struct vec*)a;
	struct vec* vb = (struct vec*)b;
	if ((va->x >= 0) != (vb->x >= 0)) // one in right half plane, other in left half
		return vb->x - va->x;
	int cross = cross_prod(va, vb);
	if (cross == 0) // va and vb point in same direction: sort on length
		return (va->x * va->x + va->y * va->y) - (vb->x * vb->x + vb->y * vb->y);
	return -cross;
}

void shoot_asteroids(struct grid* g, int x0, int y0, int nth, int* xnth, int* ynth) {
	// last 3 params just to provide puzzle answer...

	// transform asteroids into dx,dy vectors
	struct vec* vecs = malloc(g->gridw * g->gridh * sizeof(struct vec)); // too big but who cares
	int nr_vecs = 0;
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			if ((x != x0 || y != y0) && g->grid[idx].asteroid) {
				vecs[nr_vecs].x = x - x0;
				vecs[nr_vecs].y = y - y0;
				vecs[nr_vecs].exists = true;
				++nr_vecs;
			}
		}
	}
	// sort according to `angle`
	qsort(vecs, nr_vecs, sizeof(struct vec), cmp_vec);
	int vecs_left = nr_vecs;
	int idx = 0;
	while (vecs_left) {
		if (nr_vecs - vecs_left + 1 == nth) {
			*xnth = x0 + vecs[idx].x;
			*ynth = y0 + vecs[idx].y;
		}
		// printf("%d: Shooting %d,%d\n", nr_vecs - vecs_left + 1, x0 + vecs[idx].x, y0 + vecs[idx].y);
		vecs[idx].exists = false;
		--vecs_left;
		if (vecs_left) {
			// find next vec: cannot be same direction, unless no other dir exists
			int nidx = (idx + 1) % nr_vecs;
			while (nidx != idx) {
				if (vecs[nidx].exists && cross_prod(&vecs[idx], &vecs[nidx]) != 0)
					break;
				nidx = (nidx + 1) % nr_vecs;
			}
			if (nidx == idx) { // nothing found before: take next existing vec
				while (!vecs[nidx].exists)
					nidx = (nidx + 1) % nr_vecs;
			}
			idx = nidx;
		}
	}
	free(vecs);
}

void print_vecs(struct vec* v, int n) {
	for (int ii = 0; ii < n; ++ii)
		printf("(%d, %d)\n", v[ii].x, v[ii].y);
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		grid_add_line(grid, line);
	free(line);

	int x;
	int y;
	find_coordinates(grid, &x, &y); // 10A solution coordinates
	int x200, y200;
	shoot_asteroids(grid, x, y, 200, &x200, &y200);
	printf("%d\n", x200 *100 + y200);

	destroy_grid(grid);
	return 0;
}
