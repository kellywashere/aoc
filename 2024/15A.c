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
	BOX,
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
		char c = line[col];
		g->grid[idx].content = c == '#' ? WALL :
		                       c == 'O' ? BOX :
		                       EMPTY;
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
				printf("%c", cell.content == WALL ? '#' : cell.content == BOX ? 'O' : '.');
			}
		}
		printf("\n");
	}
}

bool is_in_grid(struct grid* g, int row, int col) {
	return (col >= 0 && col < g->w && row >= 0 && row < g->h);
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

void move_robot(struct grid* g, int dx, int dy) {
	int xn = g->robotx + dx;
	int yn = g->roboty + dy;
	int idxn = xn + yn * g->w;
	assert(is_in_grid(g, yn, xn));
	if (g->grid[idxn].content == WALL)
		return;
	if (g->grid[idxn].content == BOX) {
		int xe = xn + dx;
		int ye = yn + dy;
		int idxe = xe + ye * g->w;
		while (g->grid[idxe].content == BOX) {
			xe += dx;
			ye += dy;
			idxe = xe + ye * g->w;
		}
		if (g->grid[idxe].content == WALL)
			return;
		g->grid[idxe].content = BOX;
		g->grid[idxn].content = EMPTY;
	}
	g->robotx = xn;
	g->roboty = yn;
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
	// show_grid(grid);

	while (getline(&line, &len, stdin) != -1) {
		const char* l = line;
		int dir;
		while (read_move(&l, &dir)) {
			move_robot(grid, dir2dx[dir], dir2dy[dir]);
		}
	}
	//show_grid(grid);

	int total = 0;
	for (int row = 0; row < grid->h; ++row) {
		for (int col = 0; col < grid->w; ++col) {
			int idx = col + row * grid->w;
			if (grid->grid[idx].content == BOX)
				total += 100 * row + col;
		}
	}
	printf("%d\n", total);

	destroy_grid(grid);
	free(line);
	return 0;
}
