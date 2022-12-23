#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)<0?(-(x)):(x))

#define MAX_CIRCLES 64
#define SEARCH_SIZE 4000000
//#define SEARCH_SIZE 20

struct circle { // `circle` for manchester distance r, centered at x,y
	int             x;
	int             y;
	int             r;
};

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	if (isdigit(*line) && line > *pLine && *(line - 1) == '-')
		isneg = true;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? (-num) : num;
}

bool circle_covers_point(struct circle* circle, int x, int y) {
	return (ABS(x - circle->x) + ABS(y - circle->y) <= circle->r);
}

bool circle_covers_square(struct circle* circle, int x0, int y0, int x1, int y1) {
	return (circle_covers_point(circle, x0, y0) &&
			circle_covers_point(circle, x0, y1) &&
			circle_covers_point(circle, x1, y0) &&
			circle_covers_point(circle, x1, y1));
}

bool search_beacon(struct circle* circles, int nr_circles, int x0, int y0, int x1, int y1,
		/* out: */ int* xfound, int* yfound) {
	// binary search like algo, looking at square x0..x1, y0..y1
	if (x0 > x1 || y0 > y1)
		return false;

	// printf("Looking in (%d,%d)..(%d,%d)\n", x0, y0, x1, y1);

	// check if square fully covered by any of the circles. If so, return false. If not, recurse
	for (int c = 0; c < nr_circles; ++c) {
		if (circle_covers_square(&circles[c], x0, y0, x1, y1))
			return false;
	}
	if (x0 == x1 && y0 == y1) {
		*xfound = x0;
		*yfound = y0;
		return true;
	}
	int xm = (x0 + x1)/2;
	int ym = (y0 + y1)/2;
	bool found = search_beacon(circles, nr_circles, x0, y0, xm, ym, xfound, yfound);
	found = found || search_beacon(circles, nr_circles, xm + 1, y0, x1, ym, xfound, yfound);
	found = found || search_beacon(circles, nr_circles, x0, ym + 1, xm, y1, xfound, yfound);
	found = found || search_beacon(circles, nr_circles, xm + 1, ym + 1, x1, y1, xfound, yfound);
	return found;
}

int main(int argc, char* argv[]) {
	struct circle circles[MAX_CIRCLES];
	int nr_circles = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int sx = read_int(&l);
		int sy = read_int(&l);
		int bx = read_int(&l);
		int by = read_int(&l);
		int dist = ABS(sx - bx) + ABS(sy - by);
		circles[nr_circles].x = sx;
		circles[nr_circles].y = sy;
		circles[nr_circles].r = dist;
		++nr_circles;
	}
	free(line);

	int x = 0;
	int y = 0;
	search_beacon(circles, nr_circles, 0, 0, SEARCH_SIZE, SEARCH_SIZE, &x, &y);
	uint64_t p = x;
	p *= 4000000;
	p += y;
	printf("%"PRIu64"\n", p);

	return 0;
}
