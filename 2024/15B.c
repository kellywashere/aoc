#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <assert.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

enum content {
	EMPTY, /* or robot */
	WALL,
	BOX_LEFTSIDE,
	BOX_RIGHTSIDE
};

struct cell {
	enum content content;
};

struct grid {
	int            w;
	int            h;
	int            rowcapacity; // max nr of rows reserved in mem
	struct cell*   grid;

	int            robotx;
	int            roboty;
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

/* *******************************************************************************
* NOTE: This version of grid_add_line inserts 2 cells per line character!!       *
*********************************************************************************/
void grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (line[lw] && !isspace(line[lw]))
		++lw;
	lw *= 2; // DOUBLING LINE WIDTH FOR 15B
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
	for (int col = 0; col < g->w; col += 2) {
		int idx = col + g->h * g->w;
		char c = line[col/2];
		switch (c) {
			case '#':
				g->grid[idx].content     = WALL;
				g->grid[idx + 1].content = WALL;
				break;
			case 'O':
				g->grid[idx].content     = BOX_LEFTSIDE;
				g->grid[idx + 1].content = BOX_RIGHTSIDE;
				break;
			default:
				g->grid[idx].content     = EMPTY;
				g->grid[idx + 1].content = EMPTY;
		};
		if (c == '@') {
			g->robotx = col;
			g->roboty = g->h;
		}
	}
	++g->h;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->h; ++y) {
		for (int x = 0; x < g->w; ++x) {
			if (x == g->robotx && y == g->roboty)
				printf(RED "@" COL_RESET);
			else {
				struct cell cell = g->grid[y * g->w + x];
				printf("%c", cell.content == WALL ? '#' :
				             cell.content == BOX_LEFTSIDE  ? '[' :
				             cell.content == BOX_RIGHTSIDE ? ']' : '.');
			}
		}
		printf("\n");
	}
	printf("\n");
}

bool is_in_grid(struct grid* g, int x, int y) {
	return (x >= 0 && x < g->w && y >= 0 && y < g->h);
}

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

const int dir2dx[] = { 0, 1, 0, -1 };
const int dir2dy[] = {-1, 0, 1,  0 };

bool read_move(const char** pLine, int* dir) {
	char* dirstr = "^>v<";
	const char* line = *pLine;

	while (*line && strchr(dirstr, *line) == NULL)
		++line;
	if (*line == '\0')
		return false;
	*dir = strchr(dirstr, *line) - dirstr;
	++line;
	*pLine = line;
	return true;
}

// recursivele checks if box up or down movement possible
// does not actually move yet
bool can_move_box_ud(struct grid* g, int x, int y, int dy) {
	// x,y points at left side of block!
	// dy == +1 / -1
	int idx = x + y * g->w;
	assert(g->grid[idx].content == BOX_LEFTSIDE);

	// cells above/below box:
	int xn1 = x;
	int xn2 = x + 1;
	int yn = y + dy;
	enum content cont1 = g->grid[xn1 + yn * g->w].content;
	enum content cont2 = g->grid[xn2 + yn * g->w].content;

	if (cont1 == WALL || cont2 == WALL)
		return false;

	if (cont1 == BOX_LEFTSIDE && !can_move_box_ud(g, xn1, yn, dy))
		return false;

	if (cont1 == BOX_RIGHTSIDE && !can_move_box_ud(g, xn1 - 1, yn, dy))
		return false;

	if (cont2 == BOX_LEFTSIDE && !can_move_box_ud(g, xn2, yn, dy))
		return false;

	return true;
}

bool move_box_ud(struct grid* g, int x, int y, int dy) {
	// x,y points at left side of the box
	// dy == +1 / -1
	if (!can_move_box_ud(g, x, y, dy))
		return false;
	// cells above box:
	int xn1 = x;
	int xn2 = x + 1;
	int yn = y + dy;
	enum content cont1 = g->grid[xn1 + yn * g->w].content;
	enum content cont2 = g->grid[xn2 + yn * g->w].content;

	if (cont1 == BOX_LEFTSIDE)
		move_box_ud(g, xn1, yn, dy);

	if (cont1 == BOX_RIGHTSIDE)
		move_box_ud(g, xn1 - 1, yn, dy);

	if (cont2 == BOX_LEFTSIDE)
		move_box_ud(g, xn2, yn, dy);

	// now move this box
	g->grid[xn1 + yn * g->w].content = g->grid[xn1 + y * g->w].content;
	g->grid[xn1 + y * g->w].content = EMPTY;
	g->grid[xn2 + yn * g->w].content = g->grid[xn2 + y * g->w].content;
	g->grid[xn2 + y * g->w].content = EMPTY;
	return true;
}

// recursivele move box left or right, return true if successful
bool move_box_lr(struct grid* g, int x, int y, int dx) {
	// x,y points at side of the box that is being pushed
	// dx == +1 / -1
	int xn = x + 2*dx; // 2 cells over
	int idxn = xn + y * g->w;
	assert(is_in_grid(g, xn, y));
	enum content contn = g->grid[idxn].content;
	if (contn == WALL) // can't push through walls
		return false;
	if (contn == BOX_LEFTSIDE || contn == BOX_RIGHTSIDE) { // next to another box
		if (!move_box_lr(g, xn, y, dx))
			return false;
	}
	assert(g->grid[idxn].content == EMPTY);
	// move box by dx
	g->grid[idxn].content = g->grid[idxn - dx].content;
	g->grid[idxn - dx].content = g->grid[idxn - 2 * dx].content;
	g->grid[idxn - 2 * dx].content = EMPTY;
	return true;
}

void move_robot_ud(struct grid* g, int dy) {
	int x = g->robotx;
	int yn = g->roboty + dy;
	int idxn = x + yn * g->w;
	if (g->grid[idxn].content == WALL)
		return;
	if (g->grid[idxn].content == BOX_LEFTSIDE)
		if (!move_box_ud(g, x, yn, dy))
			return;
	if (g->grid[idxn].content == BOX_RIGHTSIDE)
		if (!move_box_ud(g, x - 1, yn, dy))
			return;
	g->roboty = yn;
}

void move_robot_lr(struct grid* g, int dx) {
	int xn = g->robotx + dx;
	int y = g->roboty;
	int idxn = xn + y * g->w;
	if (g->grid[idxn].content == WALL)
		return;
	if (g->grid[idxn].content == BOX_LEFTSIDE || g->grid[idxn].content == BOX_RIGHTSIDE) {
		if (!move_box_lr(g, xn, y, dx))
			return;
	}
	g->robotx = xn;
}


void move_robot(struct grid* g, int dx, int dy) {
	if (dy == 0)
		move_robot_lr(g, dx);
	else
		move_robot_ud(g, dy);
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		grid_add_line(grid, line);
	}
	//show_grid(grid);

	while (getline(&line, &len, stdin) != -1) {
		const char* l = line;
		int dir;
		while (read_move(&l, &dir)) {
			move_robot(grid, dir2dx[dir], dir2dy[dir]);
		}
	}
	// show_grid(grid);

	int total = 0;
	for (int row = 0; row < grid->h; ++row) {
		for (int col = 0; col < grid->w; ++col) {
			int idx = col + row * grid->w;
			if (grid->grid[idx].content == BOX_LEFTSIDE)
				total += 100 * row + col;
		}
	}
	printf("%d\n", total);

	destroy_grid(grid);
	free(line);
	return 0;
}
