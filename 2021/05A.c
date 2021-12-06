#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct line {
	int          x0;
	int          y0;
	int          x1;
	int          y1;
	struct line* next;
};

void order(int* a0, int* a1) {
	if (*a0 > *a1) {
		int t = *a0;
		*a0 = *a1;
		*a1 = t;
	}
}

int main(int argc, char* argv[]) {
	int x0, y0, x1, y1, x, y;
	int xmax = 0;
	int ymax = 0;
	struct line* lines = NULL;
	struct line* l;
	int* grid;
	while (scanf("%d,%d -> %d,%d", &x0, &y0, &x1, &y1) != EOF) {
		if (x0 != x1 && y0 != y1)
			continue;
		// add to lines linked list
		l = malloc(sizeof(struct line));
		order(&x0, &x1);
		order(&y0, &y1);
		l->x0 = x0;
		l->y0 = y0;
		l->x1 = x1;
		l->y1 = y1;
		l->next = lines;
		lines = l;
		// max coordinates
		if (x0 > xmax)
			xmax = x0;
		if (x1 > xmax)
			xmax = x1;
		if (y0 > ymax)
			ymax = y0;
		if (y1 > ymax)
			ymax = y1;
	}
	// create grid
	int rows = ymax + 1;
	int cols = xmax + 1;
	grid = calloc(rows * cols, sizeof(int));
	// process lines
	l = lines;
	while (l) {
		if (l->x0 == l->x1) {
			for (y = l->y0; y <= l->y1; ++y)
				++grid[y * cols + l->x0];
		}
		if (l->y0 == l->y1) {
			for (x = l->x0; x <= l->x1; ++x)
				++grid[l->y0 * cols + x];
		}
		l = l->next;
	}
	// count
	int count = 0;
	for (y = 0; y <= ymax; ++y) {
		for (x = 0; x <= xmax; ++x) {
			if (grid[y * cols + x] >= 2)
				++count;
		}
	}
	printf("%d\n", count);

	//clean-up
	free(grid);
	while (lines) {
		l = lines;
		lines = lines->next;
		free(l);
	}

	return 0;
}
