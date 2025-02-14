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

#define ABS(x) ((x)<0?(-(x)):(x))

struct vec2 {
	int x;
	int y;
};

struct vec2_list {
	struct vec2       pos;
	struct vec2_list* next;
};

struct cell {
	char content;
};

struct grid {
	int           w;
	int           h;
	int           rowcapacity; // max nr of rows reserved in mem
	struct cell*  grid;
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

int* get_col_widths(struct grid* g) {
	int* colw = malloc(g->w * sizeof(int));
	for (int col = 0; col < g->w; ++col) {
		bool empty = true;
		for (int row = 0; empty && row < g->h; ++row) {
			empty = empty && g->grid[col + row * g->w].content != '#';
		}
		colw[col] = empty ? 2 : 1;
	}
	return colw;
}

struct vec2_list* get_galaxy_pos(struct grid* g) {
	struct vec2_list* poslist = NULL;
	int* colw = get_col_widths(g);
	int ypos = 0;
	for (int y = 0; y < g->h; ++y) {
		int xpos = 0;
		bool emptyrow = true;
		for (int x = 0; x < g->w; ++x) {
			if (g->grid[y * g->w + x].content == '#') {
				emptyrow = false;
				struct vec2_list* el = malloc(sizeof(struct vec2_list));
				el->pos.x = xpos;
				el->pos.y = ypos;
				el->next = poslist;
				poslist = el;
			}
			xpos += colw[x];
		}
		ypos += emptyrow ? 2 : 1;
	}
	free(colw);
	return poslist;
}

/*
void print_expanded_grid(struct grid* g) {
	int* colw = get_col_widths(g);
	for (int y = 0; y < g->h; ++y) {
		bool emptyrow = true;
		for (int x = 0; x < g->w; ++x) {
			char c = g->grid[y * g->w + x].content;
			if (c == '#') {
				emptyrow = false;
				printf("#");
			}
			else {
				for (int ii = 0; ii < colw[x]; ++ii)
					printf(".");
			}
		}
		printf("\n");
		if (emptyrow) {
			for (int x = 0; x < g->w; ++x)
				for (int ii = 0; ii < colw[x]; ++ii)
					printf(".");
			printf("\n");
		}
	}
	free(colw);
}
*/

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	//show_grid(grid);
	//print_expanded_grid(grid);


	struct vec2_list* poslist = get_galaxy_pos(grid);

	/*
	for (struct vec2_list* el = poslist; el; el = el->next) {
		printf("%d,%d\n", el->pos.x, el->pos.y);
	}
	*/

	int sum = 0;
	for (struct vec2_list* el1 = poslist; el1; el1 = el1->next) {
		for (struct vec2_list* el2 = el1->next; el2; el2 = el2->next) {
			int dx = el1->pos.x - el2->pos.x;
			int dy = el1->pos.y - el2->pos.y;
			int dist = ABS(dx) + ABS(dy);
			//printf("Dist between (%d,%d) and (%d,%d): %d\n", el1->pos.x, el1->pos.y, el2->pos.x, el2->pos.y, dist);
			sum += dist;
		}
	}
	printf("%d\n", sum);

	destroy_grid(grid);
	free(poslist);
	return 0;
}
