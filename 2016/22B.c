#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// I first solved this one by hand!
// There is already one empty cell in starting grid
// Only possbile moves from start are 4 moves into that hole
// Strategy:
// 1. Move hole to left of cell containing goal data
// 2. Move goal data into hole
// 3. Use 4 moves to move hole down, left, left, up so it
// is again to the left of the data.
// [Note this only works if there is no "wall" in top row
// (which there isn't) otherwise we would have to move the hole
// underneath the goal data, and move goal data downward]
// So once the hole is to the left of the data, and there is no
// wall in top two rows, it takes another 5*(dist goal to target) + 1
// move. This is what an A* heuristic could be based on.
//
// Solution can be made much more robust
// e.g. if there is no initial hole: create one first, as close as possible to goal data
// maneuvre around walls in top two rows, etc.

// For Astar I am not using minqueue, to make code simpler (but of course slower)

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && (*line < '0' || *line > '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

enum cell {
	HOLE,
	WALL,
	GOALDATA,
	DATA
};

struct drive {
	int used;
	int size;
	bool is_movable; // wall detection algo using flood fill
};

void show_drivegrid(struct drive* g, int w, int h) {
	for (int y = 0; y < h; ++y) {
		/*
		for (int x = 0; x < w; ++x)
			printf(" %d,%d", x%10,y%10);
		printf("\n");
		*/
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].used);
		printf("\n");
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].size - g[x + w * y].used); // avail
		printf("\n");
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].is_movable);
		printf("\n\n");
	}
}

void show_grid(enum cell* g, int w, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			enum cell c = g[x + y*w];
			printf("%c", c == HOLE ? 'x' :
			             c == WALL ? '#' :
			             c == GOALDATA ? 'G' :
			             '.');
		}
		printf("\n");
	}
}

void floodfill_hole_rec(struct drive* g, int w, int h, int x, int y) {
	int idx = x + w * y;
	g[idx].is_movable = true;
	if (x > 0 && g[idx - 1].is_movable == 0 && g[idx - 1].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x - 1, y);
	if (x < w - 1 && g[idx + 1].is_movable == 0 && g[idx + 1].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x + 1, y);
	if (y > 0 && g[idx - w].is_movable == 0 && g[idx - w].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x, y - 1);
	if (y < h - 1 && g[idx + w].is_movable == 0 && g[idx + w].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x, y + 1);
}

// floodfill finds out where the hole can travel to (part of wall detection)
void floodfill_hole(struct drive* g, int w, int h) {
	// init for flood fill
	for (int ii = 0; ii < w * h; ++ii)
		g[ii].is_movable = g[ii].used == 0 ? true : false; // set hole(s) to true
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (g[x + w*y].used == 0)
				floodfill_hole_rec(g, w, h, x, y);
		}
	}
}

/*
// Astar
struct Astarnode {
	int gscore;
	int fscore;
	int holex;
	int holey;
	int goalx;
	int goaly;
*/

// struct tmpnode only used for temp storage at data read in
struct tmpnode {
	int          x;
	int          y;
	int          size;
	int          used;
	struct tmpnode* next;
};

int main(int argc, char* argv[]) {
	struct tmpnode* tmpnodelist = NULL;
	int gridw = 0;
	int gridh = 0;

	// read stdin
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (line[0] == '/') {
			struct tmpnode* n = malloc(sizeof(struct tmpnode));
			char* l = line;
			n->x = read_int(&l);
			n->y = read_int(&l);
			n->size = read_int(&l);
			n->used = read_int(&l);
			n->next = tmpnodelist;
			tmpnodelist = n;
			gridw = (n->x+1) > gridw ? (n->x+1) : gridw;
			gridh = (n->y+1) > gridh ? (n->y+1) : gridh;
		}
	}
	free(line);

	// transform LL into temporary "drive grid"
	struct drive* drivegrid = calloc(gridw * gridh, sizeof(struct drive));
	for (struct tmpnode* n = tmpnodelist; n != NULL; n = n->next) {
		drivegrid[n->x + gridw * n->y].used = n->used;
		drivegrid[n->x + gridw * n->y].size = n->size;
	}
	while (tmpnodelist) {
		struct tmpnode* n = tmpnodelist;
		tmpnodelist = tmpnodelist->next;
		free(n);
	}

	floodfill_hole(drivegrid, gridw, gridh); // wall detection via floodfill
	// show_drivedrivegrid(drivegrid, gridw, gridh);

	// transform into puzzle drivegrid
	enum cell* grid = calloc(gridw * gridh, sizeof(enum cell));
	for (int ii = 0; ii < gridw * gridh; ++ii) {
		grid[ii] = ii == gridw - 1 ? GOALDATA :
		           drivegrid[ii].used == 0 ? HOLE :
		           !drivegrid[ii].is_movable ? WALL :
		           DATA;
	}
	free(drivegrid);

	show_grid(grid, gridw, gridh);


	free(grid);
	return 0;
}
