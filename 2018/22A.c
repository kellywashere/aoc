#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MOD 20183

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

void read_inputs(int* depth, int* targetx, int* targety) {
	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	char* l = line;
	*depth = read_int(&l);

	getline(&line, &len, stdin);
	l = line;
	*targetx = read_int(&l);
	*targety = read_int(&l);
	free(line);
}

struct cell {
	int geoidx;
	int type;
};

struct grid {
	int          gridw;
	int          gridh;
	int          targetx;
	int          targety;
	int          depth;
	struct cell* grid;
};

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = w;
	g->gridh = h;
	g->grid = calloc(w * h, sizeof(struct cell));
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void calc_geoidx_and_type(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			int geoidx;
			if (idx == 0)
				geoidx = 0;
			else if (x == g->targetx && y == g->targety)
				geoidx = 0;
			else if (y == 0)
				geoidx = (x * 16807) % MOD;
			else if (x == 0)
				geoidx = (y * 48271) % MOD;
			else {
				int el1 = (g->grid[idx - 1].geoidx + g->depth) % MOD;
				int el2 = (g->grid[idx - g->gridw].geoidx + g->depth) % MOD;
				geoidx = (el1 * el2) % MOD;
			}
			g->grid[idx].geoidx = geoidx;
			int el = ((geoidx + g->depth) % MOD);
			g->grid[idx].type = el % 3;
		}
	}
}

void show_grid_type(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			int type = g->grid[idx].type;
			putchar(type == 0 ? '.' : type == 1 ? '=' : '|');
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[]) {
	int depth = 0;
	int targetx = 0;
	int targety = 0;
	read_inputs(&depth, &targetx, &targety);

	struct grid* grid = create_grid(targetx + 1, targety + 1);
	grid->depth = depth;
	grid->targetx = targetx;
	grid->targety = targety;

	calc_geoidx_and_type(grid);

	// show_grid_type(grid);

	// puzzle answer:
	int sum = 0;
	for (int y = 0; y < grid->gridh; ++y)
		for (int x = 0; x < grid->gridw; ++x)
			sum += grid->grid[y * grid->gridw + x].type;
	printf("%d\n", sum);

	destroy_grid(grid);
	return 0;
}
