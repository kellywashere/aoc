#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_PARTICLES 512

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

struct particle {
	int x;
	int y;
	int vx;
	int vy;
};

int mandist(int dx, int dy) { // simpler than euclidean dist
	return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

int main(int argc, char* argv[]) {
	struct particle p[MAX_PARTICLES];
	int nr_p = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		p[nr_p].x = read_int(&l);
		p[nr_p].y = read_int(&l);
		p[nr_p].vx = read_int(&l);
		p[nr_p].vy = read_int(&l);
		++nr_p;
	}
	free(line);


	// algo can be sped up a lot by:
	// 1. first finding two most distant points
	// 2. finding time at which these points reach min dist
	// run next algo starting from that time minus some offset (e.g. 20)

	// find most distant points
	int p1 = 0, p2 = 0;
	int dmax = 0;
	for (int ii = 0; ii < nr_p - 1; ++ii) {
		for (int jj = ii + 1; jj < nr_p; ++jj) {
			int d = mandist(p[ii].x - p[jj].x, p[ii].y - p[jj].y);
			if (d > dmax) {
				dmax = d;
				p1 = ii;
				p2 = jj;
			}
		}
	}
	// calc at what time these points reach minimum distance to each other
	// here we use euclidean dist, easier to differentiate...
	int x0 = p[p1].x - p[p2].x;
	int y0 = p[p1].y - p[p2].y;
	int vx = p[p1].vx - p[p2].vx;
	int vy = p[p1].vy - p[p2].vy;
	// we differentiate (x0+vx*t)^2 + (y0+vy*t)^2, and set to 0
	int tmin = -(x0*vx + y0*vy)/(vx*vx + vy*vy);

	int d_prev = INT_MAX;
	int d = INT_MAX;
	int t;
	// keep increasing time, and calc'ing total distance of each particle to its nearest neighbor
	// until that distance no longer decreases
	for (t = tmin - 20; d <= d_prev; ++t) {
		d_prev = d;
		int dtot = 0;
		// calc dist of each particle to nearest neighbor
		for (int ii = 0; ii < nr_p - 1; ++ii) {
			int dmin = INT_MAX;
			// find nearest neighbor distance
			for (int jj = ii + 1; jj < nr_p; ++jj) {
				int dx = (p[ii].x + t * p[ii].vx) - (p[jj].x + t * p[jj].vx);
				int dy = (p[ii].y + t * p[ii].vy) - (p[jj].y + t * p[jj].vy);
				int d = mandist(dx, dy);
				dmin = d < dmin ? d : dmin;
			}
			dtot += dmin;
		}
		d = dtot;
		// printf("t = %d, dtot = %d\n", t, dtot);
	}
	int tmindist = t - 2;

	int xmin = INT_MAX;
	int xmax = 0;
	int ymin = INT_MAX;
	int ymax = 0;
	for (int ii = 0; ii < nr_p; ++ii) {
		p[ii].x += p[ii].vx * tmindist;
		p[ii].y += p[ii].vy * tmindist;
		xmin = p[ii].x < xmin ? p[ii].x : xmin;
		ymin = p[ii].y < ymin ? p[ii].y : ymin;
		xmax = p[ii].x > xmax ? p[ii].x : xmax;
		ymax = p[ii].y > ymax ? p[ii].y : ymax;
	}
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			char c = ' ';
			for (int ii = 0; ii < nr_p; ++ii)
				c = p[ii].x == x && p[ii].y == y ? '#' : c;
			printf("%c", c);
		}
		printf("\n");
	}
	return 0;
}
