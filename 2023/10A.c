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
			char c = g->grid[y * g->w + x].content;
			// printf(GRN "%c" COL_RESET, c);
			printf("%c", c);
		}
		printf("\n");
	}
}

int loop_length(struct grid* g) {
	char* north = "|LJ";
	char* east  = "-LF";
	char* south = "|F7";
	char* west  = "-J7";
	int steps = 0;
	struct vec2 pos = g->startpos;

	int idx = pos.y * g->w + pos.x;

	// find first step
	//printf("@(%d,%d)\n", pos.x, pos.y);
	if (pos.x < g->w - 1  && strchr(west, g->grid[idx + 1].content))
		++pos.x;
	else if (pos.x > 0 && strchr(east, g->grid[idx - 1].content))
		--pos.x;
	else if (pos.y < g->h - 1 && strchr(north, g->grid[idx + g->w].content))
		++pos.y;
	else if (pos.y > 0 && strchr(south, g->grid[idx + g->w].content))
		--pos.y;
	else {
		printf("Cannot find any direction to go to from S\n");
		return -1;
	}

	++steps;
	int previdx = g->startpos.y * g->w + g->startpos.x;
	while (pos.x != g->startpos.x || pos.y != g->startpos.y) {
		idx = pos.y * g->w + pos.x;
		char c = g->grid[idx].content;

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
			return -1;
		}
		++steps;
		previdx = idx;
	}
	return steps;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	//show_grid(grid);
	printf("%d\n", loop_length(grid) / 2);

	destroy_grid(grid);
	return 0;
}
