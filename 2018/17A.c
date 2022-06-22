#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define SOURCE_X 500
#define SOURCE_Y   0

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

struct clayline { // temp storage
	int left;
	int top;
	int right;
	int bot;
	struct clayline* next;
};

enum cell {
	SAND = 0,
	CLAY,
	FLOWING,
	STILL
};

struct grid {
	int        gridw;
	int        gridh;
	enum cell* grid;
	// int        sourcex;
	int        ymin;
	int        ymax;
};

struct grid* claylines_to_grid(struct clayline* lines) {
	struct grid* g = malloc(sizeof(struct grid));

	int xmin = lines->left;
	int xmax = lines->right;
	g->ymin = lines->top;
	g->ymax = lines->bot;
	// figure out gridsize
	for (struct clayline* cl = lines->next; cl; cl = cl->next) {
		xmin    = cl->left  < xmin    ? cl->left  : xmin;
		xmax    = cl->right > xmax    ? cl->right : xmax;
		g->ymin = cl->top   < g->ymin ? cl->top   : g->ymin;
		g->ymax = cl->bot   > g->ymax ? cl->bot   : g->ymax;
	}
	int dx = xmin - 1; // -1: allow 1 column for water flow to left
	g->gridw = xmax - xmin + 1 + 2; // +2: col left and col right for waterflow
	g->gridh = g->ymax + 1;
	g->grid = calloc(g->gridw * g->gridh, sizeof(enum cell)); // All SAND
	for (struct clayline* cl = lines; cl; cl = cl->next) {
		for (int y = cl->top; y <= cl->bot; ++y)
			for (int x = cl->left - dx; x <= cl->right - dx; ++x)
				g->grid[y * g->gridw + x] = CLAY;
	}
	g->grid[SOURCE_Y * g->gridw + (SOURCE_X - dx)] = FLOWING;
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}


// SAND -> FLOWING
//   if there is FLOWING above
//   if there is FLOWING to the left and CLAY/STILL left-down
//   if there is FLOWING to the right and CLAY/STILL right-down
// FLOWING -> STILL
//   if we have a horz line: C - F/S - F/S - ... - F/S - C
//   and underneath:             S/C - S/C - ... - S/C
//   then all the FLOWING in the top line become still
bool single_step(struct grid* g) {
	enum cell* gg = g->grid;
	int w = g->gridw;
	bool changed = false;
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < w; ++x) {
			int idx = y * w + x;
			if (gg[idx] == FLOWING && y < g->gridh - 1) {
				int idxb = idx + w; // below current cell
				if (gg[idxb] == SAND) {
					gg[idxb] = FLOWING;
					changed = true;
				}
				else if (gg[idxb] == CLAY || gg[idxb] == STILL) {
					for (int ii = idx; gg[ii] == FLOWING && (gg[ii + w] == CLAY || gg[ii + w] == STILL); --ii) {
						if (gg[ii - 1] == SAND) {
							gg[ii - 1] = FLOWING;
							changed = true;
						}
					}
					for (int ii = idx; gg[ii] == FLOWING && (gg[ii + w] == CLAY || gg[ii + w] == STILL); ++ii) {
						if (gg[ii + 1] == SAND) {
							gg[ii + 1] = FLOWING;
							changed = true;
						}
					}
				}
			}
		}
	}
	// turning FLOWING into STILL (tricky bit)
	for (int y = 0; y < g->gridh - 1; ++y) {
		bool in_pattern = false;
		for (int x = 0; x < w; ++x) {
			int idx = y * w + x;
			int idxb = idx + w;
			if (in_pattern && gg[idx] == CLAY) { // found one
				for (int ii = idx - 1; gg[ii] != CLAY; --ii) {
					if (gg[ii] != STILL)
						changed = true;
					gg[ii] = STILL;
				}
				in_pattern = false;
			}
			if (in_pattern && ((gg[idx] != FLOWING && gg[idx] != STILL) || (gg[idxb] != CLAY && gg[idxb] != STILL)))
				in_pattern = false;
			if (gg[idx] == CLAY)
				in_pattern = true;
		}
	}

	return changed;
}

int count_water(struct grid* g) {
	int count = 0;
	for (int y = g->ymin; y <= g->ymax; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			enum cell c = g->grid[y * g->gridw + x];
			count += (c == FLOWING || c == STILL) ? 1 : 0;
		}
	}
	return count;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			enum cell c = g->grid[y * g->gridw + x];
			printf("%c", c == SAND ? '.' : c == CLAY ? '#' : c == FLOWING ? '|' : '~');
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	struct clayline* claylines = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		struct clayline* cl = malloc(sizeof(struct clayline));
		char* l = line;
		if (line[0] == 'x') {
			cl->left  = read_int(&l);
			cl->right = cl->left;
			cl->top   = read_int(&l);
			cl->bot   = read_int(&l);
		}
		else {
			cl->top   = read_int(&l);
			cl->bot   = cl->top;
			cl->left  = read_int(&l);
			cl->right = read_int(&l);
		}
		cl->next = claylines;
		claylines = cl;
	}
	free(line);
	struct grid* grid = claylines_to_grid(claylines);
	while (claylines) {
		struct clayline* cl = claylines;
		claylines = cl->next;
		free(cl);
	}

	// show_grid(grid);
	while (single_step(grid))
		;
	// show_grid(grid);
	printf("%d\n", count_water(grid));

	destroy_grid(grid);
	return 0;
}
