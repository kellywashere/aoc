#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

struct vec2 {
	int x;
	int y;
};

struct cell {
	char content;
	bool isloop;
};

struct grid {
	int           w;
	int           h;
	int           rowcapacity; // max nr of rows reserved in mem
	struct cell*  grid;

	struct vec2   startpos;
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
		g->grid[idx].content = c;
		if (c == 'S') {
			g->startpos.x = col;
			g->startpos.y = g->h;
		}
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			int idx = y * g->w + x;
			char c = g->grid[idx].content;
			if (g->grid[idx].isloop)
				printf(GRN "%c" COL_RESET, c);
			else
				printf("%c", c);
		}
		printf("\n");
	}
}


enum dir {
	NORTH = 0,
	EAST  = 1,
	SOUTH = 2,
	WEST  = 3
};

void mark_loop(struct grid* g) {
	char* north = "|LJ";
	char* east  = "-LF";
	char* south = "|F7";
	char* west  = "-J7";

	for (int ii = 0; ii < g->w * g->h; ++ii)
		g->grid[ii].isloop = false;

	struct vec2 pos = g->startpos;

	int idx = pos.y * g->w + pos.x;
	g->grid[idx].isloop = true;

	// find first step, remember initial direction for later
	//printf("@(%d,%d)\n", pos.x, pos.y);
	enum dir dirfirst = -1;
	if (pos.x < g->w - 1  && strchr(west, g->grid[idx + 1].content)) {
		++pos.x;
		dirfirst = EAST;
	}
	else if (pos.x > 0 && strchr(east, g->grid[idx - 1].content)) {
		--pos.x;
		dirfirst = WEST;
	}
	else if (pos.y < g->h - 1 && strchr(north, g->grid[idx + g->w].content)) {
		++pos.y;
		dirfirst = SOUTH;
	}
	else if (pos.y > 0 && strchr(south, g->grid[idx + g->w].content)) {
		--pos.y;
		dirfirst = NORTH;
	}
	else {
		printf("Cannot find any direction to go to from S\n");
		return;
	}

	int startidx = g->startpos.y * g->w + g->startpos.x;
	int previdx = startidx;
	while (pos.x != g->startpos.x || pos.y != g->startpos.y) {
		idx = pos.y * g->w + pos.x;
		char c = g->grid[idx].content;
		g->grid[idx].isloop = true;

		//printf("@(%d,%d): %c\n", pos.x, pos.y, c);

		if (pos.y > 0 && idx - g->w != previdx && strchr(north, c))
			--pos.y;
		else if (pos.y < g->h - 1 && idx + g->w != previdx && strchr(south, c))
			++pos.y;
		else if (pos.x > 0 && idx - 1 != previdx && strchr(west, c))
			--pos.x;
		else if (pos.x < g->w - 1 && idx + 1 != previdx && strchr(east, c))
			++pos.x;
		else {
			printf("Cannot find any direction to go to from (%d, %d)\n", pos.x, pos.y);
			return;
		}
		previdx = idx;
	}
	// replace 'S' for inside/outside algo
	int didx = startidx - previdx;
	enum dir dirlast = didx < -1 ? NORTH : didx == -1 ? WEST : didx == 1 ? EAST : SOUTH;
	int stridx = 4 * dirfirst + dirlast;
	g->grid[startidx].content = "|JxLF-Lxx7|F7xL-"[stridx];
}

int count_in_loop(struct grid* g) {
	int count = 0;
	mark_loop(g);
	for (int y = 0; y < g->h; ++y) {
		bool inloop = false;
		char last = 0;
		for (int x = 0; x < g->w; ++x) {
			int idx = y * g->w + x;
			bool isloop = g->grid[idx].isloop;
			char c = g->grid[idx].content;

			/*
			if (idx == g->startpos.y * g->w + g->startpos.x)
				printf(YEL "%c" COL_RESET, c);
			else if (isloop)
				printf(GRN "%c" COL_RESET, c);
			else if (inloop)
				printf(RED "%c" COL_RESET, c);
			else
				printf("%c", c);
			*/


			count += inloop && !isloop ? 1 : 0;

			if (isloop) {
				if (c == 'F' || c == 'L')
					last = c;
				else if ((last == 'F' && c == 'J') || (last == 'L' && c == '7') || c == '|')
					inloop = !inloop;
			}
		}
		//printf("\n");
	}
	return count;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	/*
	mark_loop(grid);
	show_grid(grid);
	*/
	printf("%d\n", count_in_loop(grid));

	destroy_grid(grid);
	return 0;
}
