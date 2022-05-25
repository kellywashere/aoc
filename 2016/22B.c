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

struct node {
	int          x;
	int          y;
	int          size;
	int          used;
	struct node* next;
};

struct gridcell {
	int used;
	int avail;
};

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

void show_grid(struct gridcell* grid, int gridw, int gridh) {
	for (int y = 0; y < gridh; ++y) {
		for (int x = 0; x < gridw; ++x)
			printf(" %d,%d", x%10,y%10);
		printf("\n");
		for (int x = 0; x < gridw; ++x)
			printf("%4d", grid[x + gridw * y].used);
		printf("\n");
		for (int x = 0; x < gridw; ++x)
			printf("%4d", grid[x + gridw * y].avail);
		printf("\n\n");
	}
}

void show_available_moves(struct gridcell* g, int w, int h) {
	printf("Available moves:\n");
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int idx = y*w + x;
			int u = g[idx].used;
			if (u > 0) {
				if (y > 0 && g[idx-w].avail >= u)
					printf("(%d,%d)->(%d,%d)\n", x,y, x,y-1);
				if (y < h-1 && g[idx+w].avail >= u)
					printf("(%d,%d)->(%d,%d)\n", x,y, x,y+1);
				if (x > 0 && g[idx-1].avail >= u)
					printf("(%d,%d)->(%d,%d)\n", x,y, x-1,y);
				if (x < w-1 && g[idx+1].avail >= u)
					printf("(%d,%d)->(%d,%d)\n", x,y, x+1,y);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	struct node* nodelist = NULL;
	int gridw = 0;
	int gridh = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (line[0] == '/') {
			struct node* n = malloc(sizeof(struct node));
			char* l = line;
			n->x = read_int(&l);
			n->y = read_int(&l);
			n->size = read_int(&l);
			n->used = read_int(&l);
			n->next = nodelist;
			nodelist = n;
			gridw = (n->x+1) > gridw ? (n->x+1) : gridw;
			gridh = (n->y+1) > gridh ? (n->y+1) : gridh;
		}
	}
	free(line);

	// transform into grid
	struct gridcell* grid = calloc(gridw * gridh, sizeof(struct gridcell));
	for (struct node* n = nodelist; n != NULL; n = n->next) {
		grid[n->x + gridw * n->y].used = n->used;
		grid[n->x + gridw * n->y].avail = n->size - n->used;
	}
	while (nodelist) {
		struct node* n = nodelist;
		nodelist = nodelist->next;
		free(n);
	}

	show_grid(grid, gridw, gridh);
	show_available_moves(grid, gridw, gridh);

	free(grid);
	return 0;
}
