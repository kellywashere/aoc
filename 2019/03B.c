#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LINESEGS 1024

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(a) (((a)<0)?-(a):(a))

struct lineseg {
	int x0;
	int y0;
	int x1;
	int y1;
};

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

bool read_lineseg(char** pLine, struct lineseg* ls, int x0, int y0) {
	char* l = *pLine;
	while (*l && !strchr("RLDU", *l))
		++l;
	switch (*l) {
		case 'R':
			ls->x1 = x0 + read_int(&l);
			ls->y1 = y0;
			break;
		case 'L':
			ls->x1 = x0 - read_int(&l);
			ls->y1 = y0;
			break;
		case 'D':
			ls->y1 = y0 + read_int(&l);
			ls->x1 = x0;
			break;
		case 'U':
			ls->y1 = y0 - read_int(&l);
			ls->x1 = x0;
			break;
		default:
			return false;
	}
	ls->x0 = x0;
	ls->y0 = y0;
	*pLine = l;
	return true;
}

bool linesegs_cross(struct lineseg* l1, struct lineseg* l2, int* xc, int* yc) {
	// check if we have horz-vert combo
	bool horz1 = l1->y1 == l1->y0;
	bool horz2 = l2->y1 == l2->y0;
	if (!(horz1 ^ horz2))
		return false;
	if (horz2) {
		struct lineseg* t = l1;
		l1 = l2;
		l2 = t;
	}
	// now l1 is horizontal, l2 is vertical
	if (l2->x0 >= MIN(l1->x0, l1->x1) && l2->x0 <= MAX(l1->x0, l1->x1) &&
	    l1->y0 >= MIN(l2->y0, l2->y1) && l1->y0 <= MAX(l2->y0, l2->y1)) {
		*xc = l2->x0;
		*yc = l1->y0;
		return true;
	}
	return false;
}

int mandist(int dx, int dy) {
	return ABS(dx) + ABS(dy);
}

int len_lineseg(struct lineseg* ls) {
	return mandist(ls->x1 - ls->x0, ls->y1 - ls->y0);
}

int main(int argc, char* argv[]) {
	struct lineseg linesegs[2][MAX_LINESEGS];
	int nr_linesegs[2];

	char *line = NULL;
	size_t len = 0;

	for (int ii = 0; ii < 2; ++ii) {
		getline(&line, &len, stdin);
		char* l = line;
		nr_linesegs[ii] = 0;
		int curx = 0;
		int cury = 0;
		while (read_lineseg(&l, &linesegs[ii][nr_linesegs[ii]], curx, cury)) {
			curx += linesegs[ii][nr_linesegs[ii]].x1 - linesegs[ii][nr_linesegs[ii]].x0;
			cury += linesegs[ii][nr_linesegs[ii]].y1 - linesegs[ii][nr_linesegs[ii]].y0;
			++nr_linesegs[ii];
		}
	}
	free(line);

	int minsteps = INT_MAX;
	int steps[2] = {0};
	for (int ii = 0; ii < nr_linesegs[0]; ++ii) {
		steps[1] = 0;
		for (int jj = 0; jj < nr_linesegs[1]; ++jj) {
			int xc, yc;
			if (linesegs_cross(&linesegs[0][ii], &linesegs[1][jj], &xc, &yc)) {
				int s = steps[0] + steps[1];
				s += mandist(linesegs[0][ii].x0 - xc, linesegs[0][ii].y0 - yc);
				s += mandist(linesegs[1][jj].x0 - xc, linesegs[1][jj].y0 - yc);
				if (s > 0 && s < minsteps)
					minsteps = s;
			}
			steps[1] += len_lineseg(&linesegs[1][jj]);
		}
		steps[0] += len_lineseg(&linesegs[0][ii]);
	}
	printf("%d\n", minsteps);

	return 0;
}
