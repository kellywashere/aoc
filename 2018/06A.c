#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define MAX_COORDS 128

struct coord {
	int x;
	int y;
	int area;
	bool has_inf_area;
};

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

int mandist(int dx, int dy) {
	return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

int main(int argc, char* argv[]) {
	struct coord coords[MAX_COORDS];
	int nr_coords = 0;

	int minx = INT_MAX;
	int maxx = 0;
	int miny = INT_MAX;
	int maxy = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		coords[nr_coords].x = read_int(&l);
		coords[nr_coords].y = read_int(&l);
		coords[nr_coords].area = 0;
		coords[nr_coords].has_inf_area = false;
		minx = coords[nr_coords].x < minx ? coords[nr_coords].x : minx;
		maxx = coords[nr_coords].x > maxx ? coords[nr_coords].x : maxx;
		miny = coords[nr_coords].y < miny ? coords[nr_coords].y : miny;
		maxy = coords[nr_coords].y > maxy ? coords[nr_coords].y : maxy;
		++nr_coords;
	}
	// find out which points have infinite area
	for (int ii = 0; ii < nr_coords; ++ii) {
		bool inf_up = true;
		bool inf_left = true;
		bool inf_down = true;
		bool inf_right = true;
		for (int jj = 0; jj < nr_coords; ++jj) {
			if (ii == jj)
				continue;
			int dx = coords[ii].x > coords[jj].x ? coords[ii].x - coords[jj].x : coords[jj].x - coords[ii].x;
			int dy = coords[ii].y > coords[jj].y ? coords[ii].y - coords[jj].y : coords[jj].y - coords[ii].y;
			if (coords[jj].y - coords[ii].y <= -dx)
				inf_up = false;
			if (coords[jj].y - coords[ii].y >= dx)
				inf_down = false;
			if (coords[jj].x - coords[ii].x <= -dy)
				inf_left = false;
			if (coords[jj].x - coords[ii].x >= dy)
				inf_right = false;
		}
		coords[ii].has_inf_area = inf_up || inf_down || inf_left || inf_right;
	}
	// horrible algo to count area
	for (int y = miny; y <= maxy; ++y) {
		for (int x = minx; x <= maxx; ++x) {
			int mindist = INT_MAX;
			int mincoord_idx = -1;
			for (int ii = 0; ii < nr_coords; ++ii) {
				int d = mandist(x - coords[ii].x, y - coords[ii].y);
				if (d <= mindist) {
					mincoord_idx = d == mindist ? -1 : ii;
					mindist = d;
				}
			}
			if (mincoord_idx != -1)
				++coords[mincoord_idx].area;
		}
	}
	int maxarea = 0;
	for (int ii = 0; ii < nr_coords; ++ii) {
		if (!coords[ii].has_inf_area) {
			printf("%d,%d: %d\n", coords[ii].x, coords[ii].y, coords[ii].area);
			maxarea = coords[ii].area > maxarea ? coords[ii].area : maxarea;
		}
	}
	printf("%d\n", maxarea);

	free(line);
	return 0;
}
