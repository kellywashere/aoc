#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define MAX_COORDS 128

struct coord {
	int x;
	int y;
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
	int totdist_max = 10000;
	if (argc > 1)
		totdist_max = atoi(argv[1]);

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
		minx = coords[nr_coords].x < minx ? coords[nr_coords].x : minx;
		maxx = coords[nr_coords].x > maxx ? coords[nr_coords].x : maxx;
		miny = coords[nr_coords].y < miny ? coords[nr_coords].y : miny;
		maxy = coords[nr_coords].y > maxy ? coords[nr_coords].y : maxy;
		++nr_coords;
	}

	int area = 0;
	for (int y = miny; y <= maxy; ++y) {
		for (int x = minx; x <= maxx; ++x) {
			int totdist = 0;
			for (int ii = 0; ii < nr_coords; ++ii)
				totdist += mandist(x - coords[ii].x, y - coords[ii].y);
			if (totdist < totdist_max)
				++area;
		}
	}
	printf("%d\n", area);

	free(line);
	return 0;
}
