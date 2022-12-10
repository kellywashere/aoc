#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define XRANGE 1024
#define YRANGE 1024

#define NR_KNOTS 10

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)<0?(-(x)):(x))

struct pos {
	int x;
	int y;
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

char* dirname = "RDLU";
int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, 1, 0, -1};

void move_tail(struct pos* head, struct pos* tail) {
	int dx = head->x - tail->x;
	int dy = head->y - tail->y;
	if (ABS(dx) <= 1 && ABS(dy) <= 1)
		return; // nothing to do
	dx = MAX(MIN(dx, 1), -1);
	dy = MAX(MIN(dy, 1), -1);
	tail->x += dx;
	tail->y += dy;
}

void print_state(struct pos knots[]) {
	for (int y = 0; y < YRANGE; ++y) {
		for (int x = 0; x < XRANGE; ++x) {
			char c = '.';
			for (int k = 0; k < NR_KNOTS; ++k)
				if (knots[k].x == x && knots[k].y == y)
					c = k + '0';
			printf("%c", c);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct pos knots[NR_KNOTS];
	for (int ii = 0; ii < NR_KNOTS; ++ii) {
		knots[ii].x = XRANGE / 2;
		knots[ii].y = YRANGE / 2;
	}

	bool* visited = calloc(XRANGE * YRANGE, sizeof(bool));
	visited[(YRANGE / 2) * XRANGE + XRANGE / 2] = true;

	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		char* dirchr = strchr(dirname, *l);
		if (dirchr) {
			l += 2;
			int amount = read_int(&l);
			int dir = dirchr - dirname;
			for (int ii = 0; ii < amount; ++ii) {
				knots[0].x += dir2dx[dir];
				knots[0].y += dir2dy[dir];
				for (int k = 1; k < NR_KNOTS; ++k)
					move_tail(&knots[k - 1], &knots[k]);
				struct pos* tail = &knots[NR_KNOTS - 1];
				if (tail->x < 0 || tail->x >= XRANGE || tail->y < 0 || tail->y >= YRANGE) {
					fprintf(stderr, "Out of range: (%d, %d)\n", tail->x, tail->y);
					return 1;
				}
				visited[tail->y * XRANGE + tail->x] = true;
			}
			//print_state(knots);
		}
	}
	/*
	for (int y = 0; y < YRANGE; ++y) {
		for (int x = 0; x < YRANGE; ++x) {
			printf("%c", visited[y * XRANGE + x] ? '#' : '.');
		}
		printf("\n");
	}
	*/
	int count = 0;
	for (int ii = 0; ii < XRANGE * YRANGE; ++ii)
		count += visited[ii] ? 1 : 0;
	printf("%d\n", count);

	free(line);
	free(visited);
	return 0;
}
