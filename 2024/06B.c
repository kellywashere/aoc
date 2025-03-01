#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

struct vec2 {
	int x;
	int y;
};

enum direction {
	UP = 0,
	RIGHT,
	DOWN,
	LEFT
};

const char* dirstr = "^>v<";

struct cell {
	bool object;
	// we can visit a cell facing any direction. We want to keep track of all
	// to recognize repeated state
	int  visited_dirs; // OR of direction_mask
	bool orig_route; // to rememer original route
};

struct grid {
	int            w;
	int            h;
	int            rowcapacity; // max nr of rows reserved in mem
	struct cell*   grid;

	struct vec2    pos;
	enum direction dir;
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
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		char c = line[col];
		int idx = col + g->h * g->w;
		g->grid[idx].visited_dirs = false;
		if (c == '#') {
			g->grid[idx].object = true;
		}
		else if (c != '.') { // start pos
			g->pos = (struct vec2) { .x = col, .y = g->h };
			const char* dirchar = strchr(dirstr, c);
			if (dirchar == NULL)
				fprintf(stderr, "Unexpected char %c read\n", c);
			else {
				g->dir = dirchar - dirstr;
				g->grid[idx].visited_dirs = (1 << g->dir);
			}
		}
	}
	++g->h;
}

const int dir2dx[] = { 0, 1, 0, -1};
const int dir2dy[] = {-1, 0, 1,  0};

bool walk(struct grid* g) {
	// returns true if we end up in loop
	while (true) { // exit from while-loop handled in loop
		int idx_cur = g->pos.x + g->pos.y * g->h;

		int dx = dir2dx[g->dir];
		int dy = dir2dy[g->dir];
		int xn = g->pos.x + dx;
		int yn = g->pos.y + dy;

		if (xn < 0 || xn >= g->w || yn < 0 || yn >= g->h)
			return false;

		int idxn = xn + yn * g->h;

		if (g->grid[idxn].object) { // turn
			g->dir = (g->dir + 1) % 4;
		}
		else { // step
			g->pos.x = xn;
			g->pos.y = yn;
			idx_cur = idxn;
		}
		int vd = g->grid[idx_cur].visited_dirs; // remember prev value.
		g->grid[idx_cur].visited_dirs |= (1 << g->dir);
		if (vd == g->grid[idx_cur].visited_dirs) // If OR-ing dir bit does not change, we were in this state before
			return true; // loop found
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

	// Ugly brute-forcing of solution
	
	// remember start state
	struct vec2 startpos = grid->pos;
	int startdir = grid->dir;
	int guard_idx = grid->pos.x + grid->h * grid->pos.y;

	// remember original route
	walk(grid);
	grid->pos = startpos;
	grid->dir = startdir;
	for (int ii = 0; ii < grid->w * grid->h; ++ii)
		grid->grid[ii].orig_route = grid->grid[ii].visited_dirs != 0;

	int tot = 0;
	for (int obst_idx = 0; obst_idx < grid->w * grid->h; ++obst_idx) {
		if (!grid->grid[obst_idx].orig_route) continue;
		if (obst_idx == guard_idx) continue;
		if (grid->grid[obst_idx].object) continue;

		grid->grid[obst_idx].object = true;

		// clear visited information
		for (int ii = 0; ii < grid->w * grid->h; ++ii)
			grid->grid[ii].visited_dirs = 0;

		bool loopfound = walk(grid);
		tot += loopfound ? 1 : 0;

		// reset grid
		grid->grid[obst_idx].object = false;
		grid->pos = startpos;
		grid->dir = startdir;
	}
	printf("%d\n", tot);

	destroy_grid(grid);
	return 0;
}
