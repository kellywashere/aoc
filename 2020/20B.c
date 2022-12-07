#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define TILESIZE 10

#define MAX_NRTILES 256

// operations, multiple ops are done LSB first
#define FLIPUD 1
#define FLIPLR 2
#define ROTRIGHT 4

// doing: nothing, flipud, fliplr, flipud, rot, flipud, fliplr, flipud you get all configs
const int op_seq[] = {0, 1, 2, 1, 4, 1, 2, 1};
const int op_eqv[] = {0, 1, 3, 2, 6, 4, 5, 7}; // bitcoding to achieve states in op_seq starting from init

struct grid {
	int w;
	int h;
	bool* grid;
	bool* marked; // for monster marking
};

struct sol_grid {
	int w;
	int h;
	int* sol; // solution, array of idx in tiles array
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = w;
	g->h = h;
	g->grid = calloc(w * h, sizeof(bool));
	g->marked = calloc(w * h, sizeof(bool));
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->marked);
		free(g->grid);
		free(g);
	}
}

struct grid* grid_cpy(struct grid* g) {
	struct grid* cpy = malloc(sizeof(struct grid));
	cpy->w = g->w;
	cpy->h = g->h;
	cpy->grid = malloc(g->w * g->h * sizeof(bool));
	memcpy(cpy->grid, g->grid, g->w * g->h * sizeof(bool));
	return cpy;
}

void read_row(char* l, struct grid* g, int y) {
	for (int x = 0; x < g->w; ++x)
		g->grid[y * g->w + x] = l[x] == '#';
}

void print_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x)
			printf("%c", g->grid[y * g->w + x] ? '#' : '.');
		printf("\n");
	}
}

void flipud(struct grid* g) {
	for (int y = 0; y < g->h / 2; ++y) {
		for (int x = 0; x < g->w; ++x) {
			bool t = g->grid[y * g->w + x];
			g->grid[y * g->w + x] = g->grid[(g->h - 1 - y) * g->w + x];
			g->grid[(g->h - 1 - y) * g->w + x] = t;
		}
	}
}

void fliplr(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < (g->w + 1) / 2; ++x) {
			bool t = g->grid[y * g->w + x];
			g->grid[y * g->w + x] = g->grid[y * g->w + (g->w - 1 - x)];
			g->grid[y * g->w + (g->w - 1 - x)] = t;
		}
	}
}

void rotright(struct grid* g) {
	// x,y --> h-y-1, x
	bool* cpy = malloc(g->w * g->h * sizeof(bool));
	int cpy_w = g->h;
	int cpy_h = g->w;
	for (int y = 0; y < g->h; ++y)
		for (int x = 0; x < g->w; ++x)
			cpy[x * g->h + (g->h - y - 1)] = g->grid[y * g->w + x];
	memcpy(g->grid, cpy, g->w * g->h * sizeof(bool));
	g->w = cpy_w;
	g->h = cpy_h;
	free(cpy);
}

void do_operation(struct grid* g, int op) {
	if ((op & FLIPUD))
		flipud(g);
	if ((op & FLIPLR))
		fliplr(g);
	if ((op & ROTRIGHT))
		rotright(g);
}

void rotleft(struct grid* g) { // convenience fn
	flipud(g);
	fliplr(g);
	rotright(g);
}



bool match_top(struct grid* g1, struct grid* g2, int* match_op) {
	// does t2 fit to the top row of t1 (check al orientations of t2)?
	struct grid* cpy = grid_cpy(g2);
	bool found = false;
	for (int ii = 0; !found && ii < 8; ++ii) {
		do_operation(cpy, op_seq[ii]);
		// compare top row of g1 to bottom row of cpy
		bool same = true;
		for (int x = 0; same && x < g1->w; ++x)
			same = cpy->grid[(TILESIZE-1) * cpy->w + x] == g1->grid[x];
		if (same) {
			*match_op = op_eqv[ii];
			found = true;
		}
	}
	free(cpy->grid);
	free(cpy);
	return found;
}

bool find_top_match(struct grid* tiles[], int nr_tiles, int t1_idx, int* t2_idx, int* match_op) {
	// starts search at *t2_idx
	for (; *t2_idx < nr_tiles; ++(*t2_idx))
		if (t1_idx != *t2_idx && match_top(tiles[t1_idx], tiles[*t2_idx], match_op))
			return true;
	return false;
}

bool has_top_match(struct grid* tiles[], int nr_tiles, int t1_idx) {
	// returns true if any tile can be matched to top of t1_idx
	int op;
	int idx2 = 0;
	return find_top_match(tiles, nr_tiles, t1_idx, &idx2, &op);
}

int count_top_matches(struct grid* tiles[], int nr_tiles, int t1_idx, int* t2_idx, int* match_op) {
	// returns nr of tiles that can matched to top of t1_idx tile.
	// Last match found returned in t2_idx, match_op
	int idx2 = 0;
	int count = 0;
	bool found = true;
	while (found) {
		found = find_top_match(tiles, nr_tiles, t1_idx, &idx2, match_op);
		if (found) {
			++count;
			*t2_idx = idx2;
			++idx2;
		}
	}
	return count;
}

int count_matching_sides(struct grid* tiles[], int nr_tiles, int t1_idx) {
	// count how many sides of t1 can be fitted to any t2
	int matching_sides = 0;
	for (int side = 0; side < 4; ++side) { // check all 4 sides of the t1
		int match_op; //(dummy)
		for (int t2_idx = 0; t2_idx < nr_tiles; ++t2_idx) {
			if (t1_idx == t2_idx)
				continue;
			if (match_top(tiles[t1_idx], tiles[t2_idx], &match_op)) {
				++matching_sides;
				break; // !!
			}
		}
		rotright(tiles[t1_idx]);
	}
	return matching_sides;
}

struct sol_grid* solve_grid(struct grid* tiles[], int nr_tiles) {
	struct sol_grid* sol_grid = malloc(sizeof(struct sol_grid));
	sol_grid->sol = malloc(nr_tiles * sizeof(int));
	sol_grid->w = 0;
	sol_grid->h = 0;
	int sol_grid_idx = 0;

	// find corner piece
	int idx1;
	for (idx1 = 0; idx1 < nr_tiles; ++idx1)
		if (count_matching_sides(tiles, nr_tiles, idx1) == 2)
			break;
	// rotate corner piece s.t. it fits in top-left
	while (!has_top_match(tiles, nr_tiles, idx1))
		rotright(tiles[idx1]);
	while (has_top_match(tiles, nr_tiles, idx1))
		rotright(tiles[idx1]);
	sol_grid->sol[sol_grid_idx++] = idx1;

	int idx2, op;
	while (sol_grid_idx < nr_tiles) {
		// complete row
		int count;
		do {
			rotleft(tiles[idx1]); // rotate right of tiles[idx1] side to top
			count = count_top_matches(tiles, nr_tiles, idx1, &idx2, &op);
			rotright(tiles[idx1]);
			if (count > 1)
				printf("Algo too simple if more than one match found\n");
			if (count == 1) {
				do_operation(tiles[idx2], op);
				rotright(tiles[idx2]);
				sol_grid->sol[sol_grid_idx++] = idx2;
				idx1 = idx2;
			}
		} while (count == 1);
		if (sol_grid->w == 0) {
			sol_grid->w = sol_grid_idx;
			sol_grid->h = nr_tiles / sol_grid->w;
		}
		// find first tile of next row
		if (sol_grid_idx < nr_tiles) {
			idx1 = sol_grid->sol[sol_grid_idx - sol_grid->w];
			flipud(tiles[idx1]);
			count = count_top_matches(tiles, nr_tiles, idx1, &idx2, &op);
			flipud(tiles[idx1]);
			if (count > 1)
				printf("Algo too simple if more than one match found\n");
			do_operation(tiles[idx2], op);
			flipud(tiles[idx2]);
			sol_grid->sol[sol_grid_idx++] = idx2;
			idx1 = idx2;
		}
	}
	return sol_grid;
}

bool is_overlapping(struct grid* g, struct grid* m, int xoffs, int yoffs) {
	for (int y = 0; y < m->h; ++y) {
		for (int x = 0; x < m->w; ++x) {
			if (m->grid[y * m->w + x] && !g->grid[(yoffs + y) * g->w + (xoffs + x)])
				return false;
		}
	}
	return true;
}

void mark_overlaps(struct grid* g, struct grid* m) {
	for (int yoffs = 0; yoffs <= g->h - m->h; ++yoffs) {
		for (int xoffs = 0; xoffs <= g->w - m->w; ++xoffs) {
			if (is_overlapping(g, m, xoffs, yoffs)) {
				for (int y = 0; y < m->h; ++y)
					for (int x = 0; x < m->w; ++x)
						if (m->grid[y * m->w + x])
							g->marked[(yoffs + y) * g->w + (xoffs + x)] = true;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct grid* tiles[MAX_NRTILES];
	int nr_tiles = 0;

	int rows_read = 0;

	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line))
			continue;
		if (l[0] == 'T')
			continue;
		else {
			if (rows_read == 0)
				tiles[nr_tiles] = create_grid(TILESIZE, TILESIZE);
			read_row(l, tiles[nr_tiles], rows_read);
			++rows_read;
			if (rows_read == TILESIZE) {
				++nr_tiles;
				rows_read = 0;
			}
		}
	}
	free(line);

	struct sol_grid* sol_grid = solve_grid(tiles, nr_tiles);

	// create search grid
	struct grid* search_grid = create_grid(sol_grid->w * (TILESIZE - 2), sol_grid->h * (TILESIZE - 2));
	for (int y = 0; y < search_grid->h; ++y) {
		for (int x = 0; x < search_grid->w; ++x) {
			int idx = (y / (TILESIZE-2)) * sol_grid->w + (x / (TILESIZE-2));
			struct grid* tile = tiles[sol_grid->sol[idx]];
			int tile_y = 1 + (y % (TILESIZE - 2));
			int tile_x = 1 + (x % (TILESIZE - 2));
			search_grid->grid[y * search_grid->w + x] = tile->grid[tile_y * tile->w + tile_x];
		}
	}

	struct grid* monster = create_grid(20, 3);
	read_row("                  # ", monster, 0);
	read_row("#    ##    ##    ###", monster, 1);
	read_row(" #  #  #  #  #  #   ", monster, 2);

	for (int ii = 0; ii < 8; ++ii) {
		do_operation(monster, op_seq[ii]);
		mark_overlaps(search_grid, monster);
	}

	// count final answer
	int count = 0;
	for (int ii = 0; ii < search_grid->w * search_grid->h; ++ii)
		count += (search_grid->grid[ii] && !search_grid->marked[ii]) ? 1 : 0;
	printf("%d\n", count);

	// clean up
	for (int ii = 0; ii < nr_tiles; ++ii)
		destroy_grid(tiles[ii]);
	destroy_grid(search_grid);
	destroy_grid(monster);

	free(sol_grid->sol);
	free(sol_grid);

	return 0;
}
