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

struct cell {
	char type;
	bool visited;
};

struct grid {
	int            w;
	int            h;
	int            rowcapacity; // max nr of rows reserved in mem
	struct cell*   grid;
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
		int idx = col + g->h * g->w;
		g->grid[idx].type = line[col];
		g->grid[idx].visited = false;
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			struct cell cell = g->grid[y * g->w + x];
			if (cell.visited)
				printf(GRN "%c" COL_RESET, cell.type);
			else
				printf("%c", cell.type);
		}
		printf("\n");
	}
}

void grid_reset_visited(struct grid* g) {
	for (int ii = 0; ii < g->w * g->h; ++ii)
		g->grid[ii].visited = false;
}

bool grid_find_next_unvisited(struct grid* g, int* idx) {
	for (int ii = *idx; ii < g->w * g->h; ++ii) {
		if (!g->grid[ii].visited) {
			*idx = ii;
			return true;
		}
	}
	return false;
}

// clockwise, up, right, down, left
const int dir2dx[] = { 0, 1, 0, -1 };
const int dir2dy[] = {-1, 0, 1,  0 };

bool is_in_grid(struct grid* g, int row, int col) {
	return (col >= 0 && col < g->w && row >= 0 && row < g->h);
}

static
void grid_area_and_corners_rec(struct grid* g, int row, int col,
		int* area, int* corners) {
	int idx = col + row * g->w;
	if (g->grid[idx].visited) // already visited
		return;
	g->grid[idx].visited = true;
	char type = g->grid[idx].type;
	++(*area);

	// count corners
	for (int dir = 0; dir < 4; ++dir) {
		int c1 = col + dir2dx[dir];
		int r1 = row + dir2dy[dir];
		bool type1_eq_type = is_in_grid(g, r1, c1) &&
			g->grid[c1 + r1 * g->w].type == type;
		int dir2 = (dir + 1) % 4;
		int c2 = col + dir2dx[dir2];
		int r2 = row + dir2dy[dir2];
		bool type2_eq_type = is_in_grid(g, r2, c2) &&
			g->grid[c2 + r2 * g->w].type == type;
		int cdiag = c1 + dir2dx[dir2];
		int rdiag = r1 + dir2dy[dir2];
		bool typediag_eq_type = is_in_grid(g, rdiag, cdiag) &&
			g->grid[cdiag + rdiag * g->w].type == type;
		if (!type1_eq_type && !type2_eq_type)
			++(*corners); // outer corner
		if (type1_eq_type && type2_eq_type && !typediag_eq_type)
			++(*corners); // inner corner
	}
	for (int dir = 0; dir < 4; ++dir) {
		int cn = col + dir2dx[dir];
		int rn = row + dir2dy[dir];
		if (is_in_grid(g, rn, cn)) {
			if (g->grid[cn + rn * g->w].type == type)
				grid_area_and_corners_rec(g, rn, cn, area, corners);
		}
	}
}

// next fn should be called by user, it sets area and perimeter to 0 first
void grid_area_and_corners(struct grid* g, int row, int col,
		int* area, int* corners) {
	*area = 0;
	*corners = 0;
	grid_area_and_corners_rec(g, row, col, area, corners);
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(grid, line);
	}
	free(line);

	int idx = 0;
	int price_tot = 0;
	grid_reset_visited(grid);
	while (grid_find_next_unvisited(grid, &idx)) {
		int row = idx / grid->w;
		int col = idx % grid->w;

		int area, corners;
		grid_area_and_corners(grid, row, col, &area, &corners);
		//printf("Region %c at r:%d,c:%d: Area = %d, Corners = %d\n", grid->grid[idx].type, row, col, area, corners);
		price_tot += area * corners;
		++idx;
	}

	//show_grid(grid);

	printf("%d\n", price_tot);

	destroy_grid(grid);
	return 0;
}
