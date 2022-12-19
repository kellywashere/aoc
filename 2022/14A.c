#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

struct vec2 {
	int x;
	int y;
};

struct line {
	struct vec2  from;
	struct vec2  to;
	struct line* next; // allow LL
};

enum cell {
	EMPTY,
	WALL,
	SAND
};

struct grid {
	int        w;
	int        h;
	int        xsource; // source of sand
	enum cell* grid;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_vec2(char** pLine, struct vec2* v) {
	while (**pLine && !isdigit(**pLine))
		++(*pLine);
	if (!isdigit(**pLine))
		return false;
	v->x = 0;
	while (isdigit(**pLine)) {
		v->x = v->x * 10 + **pLine - '0';
		++(*pLine);
	}
	while (**pLine && !isdigit(**pLine))
		++(*pLine);
	v->y = 0;
	while (isdigit(**pLine)) {
		v->y = v->y * 10 + **pLine - '0';
		++(*pLine);
	}
	return true;
}

struct grid* read_grid(FILE* fp) {
	char *ln = NULL;
	size_t len = 0;
	struct line* lines = NULL; // LL
	while (getline(&ln, &len, fp) != -1) {
		if (empty_line(ln))
			continue;
		char* l = ln;
		struct line* line = malloc(sizeof(struct line)); // LL
		read_vec2(&l, &line->from);
		read_vec2(&l, &line->to);
		line->next = lines;
		lines = line;
		struct vec2 v;
		while (read_vec2(&l, &v)) {
			line = malloc(sizeof(struct line)); // LL
			line->from = lines->to;
			line->to = v;
			line->next = lines;
			lines = line;
		}
	}
	free(ln);

	// extract xmin/xmax and ymax
	int xmin = lines->from.x;
	int xmax = xmin;
	int ymax = lines->from.y;
	for (struct line* l = lines; l; l = l->next) {
		xmin = l->from.x < xmin ? l->from.x : xmin;
		xmin =   l->to.x < xmin ?   l->to.x : xmin;
		xmax = l->from.x > xmax ? l->from.x : xmax;
		xmax =   l->to.x > xmax ?   l->to.x : xmax;
		ymax = l->from.y > ymax ? l->from.y : ymax;
		ymax =   l->to.y > ymax ?   l->to.y : ymax;
	}
	--xmin;
	++xmax; // add margin of 1

	// create grid
	struct grid* grid = malloc(sizeof(struct grid));
	grid->w = xmax - xmin + 1;
	grid->h = ymax + 1;
	grid->grid = calloc(grid->w * grid->h, sizeof(enum cell));
	for (struct line* l = lines; l; l = l->next) {
		int dx = l->to.x > l->from.x ? 1 : l->to.x < l->from.x ? -1 : 0;
		int dy = l->to.y > l->from.y ? 1 : l->to.y < l->from.y ? -1 : 0;
		int x = l->from.x;
		int y = l->from.y;
		bool done = false;
		while (!done) {
			grid->grid[y * grid->w + x - xmin] = WALL;
			done = (x == l->to.x && y == l->to.y);
			x += dx;
			y += dy;
		}
	}
	while (lines) {
		struct line* line = lines;
		lines = lines->next;
		free(line);
	}
	grid->xsource = 500 - xmin;
	return grid;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			enum cell c = g->grid[y * g->w + x];
			printf("%c", c == WALL ? '#' : c == SAND ? 'o' : '.');
		}
		printf("\n");
	}
}

bool drop_sand(struct grid* g) {
	int xsand = g->xsource;
	int ysand = 0;
	bool stable = false;
	while (ysand < g->h - 1 && !stable) {
		int idx = (ysand + 1) * g->w + xsand;
		if (g->grid[idx] == EMPTY)
			++ysand;
		else if (g->grid[idx - 1] == EMPTY) {
			++ysand;
			--xsand;
		}
		else if (g->grid[idx + 1] == EMPTY) {
			++ysand;
			++xsand;
		}
		else
			stable = true;
	}
	if (stable)
		g->grid[ysand * g->w + xsand] = SAND;
	return stable;
}

int main(int argc, char* argv[]) {
	struct grid* grid = read_grid(stdin);


	while (drop_sand(grid))
		;

	int count = 0;
	for (int ii = 0; ii < grid->w * grid->h; ++ii)
		count += grid->grid[ii] == SAND ? 1 : 0;
	printf("%d\n", count);

	free(grid->grid);
	free(grid);

	return 0;
}
