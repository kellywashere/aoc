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
	bool visited;
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
		g->grid[idx].visited = false;
		if (c == '#') {
			g->grid[idx].object = true;
		}
		else if (c != '.') { // start pos
			g->pos = (struct vec2) { .x = col, .y = g->h };
			g->grid[idx].visited = true;
			const char* dirchar = strchr(dirstr, c);
			if (dirchar == NULL)
				fprintf(stderr, "Unexpected char %c read\n", c);
			else
				g->dir = dirchar - dirstr;
		}
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			struct cell cell = g->grid[y * g->w + x];
			char c = '.';
			// printf(GRN "%c" COL_RESET, c);
			if (cell.object)
				c = '#';
			else if (x == g->pos.x && y == g->pos.y)
				c = dirstr[g->dir];
			else
				c = cell.visited ? 'x' : '.';
			printf("%c", c);
		}
		printf("\n");
	}
}

const int dir2dx[] = { 0, 1, 0, -1};
const int dir2dy[] = {-1, 0, 1,  0};

void walk(struct grid* g) {
	bool valid_idx = true;
	while (valid_idx) {
		// calc next
		int dx = dir2dx[g->dir];
		int dy = dir2dy[g->dir];
		int xn = g->pos.x + dx;
		int yn = g->pos.y + dy;
		int idxn = xn + yn * g->h;

		valid_idx = (xn >= 0 && xn < g->w && yn >= 0 && yn < g->h);
		if (valid_idx) {
			if (g->grid[idxn].object) { // turn
				g->dir = (g->dir + 1) % 4;
			}
			else { // step
				g->pos.x = xn;
				g->pos.y = yn;
				g->grid[idxn].visited = true;
			}
		}
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

	walk(grid);

	int nr_visited = 0;
	for (int ii = 0; ii < grid->w * grid->h; ++ii)
		nr_visited += grid->grid[ii].visited ? 1 : 0;
	printf("%d\n", nr_visited);

	destroy_grid(grid);
	return 0;
}
