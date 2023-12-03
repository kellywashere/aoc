#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define ABS(x) ((x)<0 ? (-(x)) : (x))

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct grid {
	int    w;
	int    h;
	int    rowcapacity; // max nr of rows reserved in mem
	char*  grid;
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
		g->grid = calloc(g->w * g->rowcapacity, sizeof(int));
	}
	if (g->h >= g->rowcapacity) {
		g->grid = realloc(g->grid, g->w * 2 * g->rowcapacity * sizeof(int));
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->w; ++col) {
		char c = line[col];
		g->grid[col + g->h * g->w] = c;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x];
			printf("%c", c);
		}
		printf("\n");
	}
}

char get_grid_cell(struct grid* g, int x, int y) {
	return (x >= 0 && x < g->w && y >= 0 && y < g->h) ? g->grid[y * g->w + x] : '.';
}

bool is_symbol(char c) {
	return !isdigit(c) && c != '.';
}

bool is_empty(char c) {
	return c == '.';
}

int read_nr(struct grid* g, int x, int y) {
	int num = 0;
	while (isdigit(get_grid_cell(g, x, y))) {
		num = 10 * num + g->grid[y * g->w + x] - '0';
		++x;
	}
	return num;
}

bool is_nr_connected_to_star(struct grid* g, int xn, int yn, int xs, int ys) {
	bool connected = false;
	if (ABS(yn - ys) <= 1) {
		while (isdigit(get_grid_cell(g, xn, yn))) {
			connected = connected || ABS(xn - xs) <= 1;
			++xn;
		}
	}
	return connected;
}

int validate_star(struct grid* g, int xs, int ys) {
	int prod = 1;
	int count = 0;
	if (get_grid_cell(g, xs, ys) == '*') { // check if connected to 2 numbers
		for (int yn = ys - 1; yn <= ys + 1; ++yn) {
			for (int xn = 0; xn <= xs + 1; ++xn) {
				char cur = get_grid_cell(g, xn, yn);
				char prev = get_grid_cell(g, xn - 1, yn);
				if (isdigit(cur) && !isdigit(prev)) { // nr starts here
					if (is_nr_connected_to_star(g, xn, yn, xs, ys)) {
						int num = read_nr(g, xn, yn);
						prod *= read_nr(g, xn, yn);
						++count;
					}
				}
			}
		}
	}
	return count == 2 ? prod : 0;
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line))
			grid_add_line(grid, line);
	}
	free(line);
	//show_grid(grid);

	int sum = 0;
	for (int ys = 0; ys < grid->h; ++ys) {
		for (int xs = 0; xs < grid->w; ++xs) {
			sum += validate_star(grid, xs, ys);
		}
	}

	printf("%d\n", sum);
	destroy_grid(grid);
	return 0;
}
