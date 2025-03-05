#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include <assert.h>

#define MAX_ROBOTS 1000
/*
#define WIDTH      11
#define HEIGHT     7
*/
#define WIDTH      101
#define HEIGHT     103

struct robot {
	int x;
	int y;
	int vx;
	int vy;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_signed_int(const char** pLine, int* x) {
	int num = 0;
	int sign = 1;
	const char* line = *pLine;
	while (*line && (!isdigit(*line) && *line != '-'))
		++line;
	if (*line == '\0')
		return false;
	if (*line == '-') {
		sign = -1;
		if (!isdigit(*(++line)))
			return false;
	}
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = sign * num;
	*pLine = line;
	return true;
}

int* robots_to_grid(struct robot robots[], size_t robots_sz) {
	int* grid = calloc(WIDTH * HEIGHT, sizeof(int));
	for (size_t ii = 0; ii < robots_sz; ++ii) {
		assert(robots[ii].x >= 0 && robots[ii].x < WIDTH);
		assert(robots[ii].y >= 0 && robots[ii].y < HEIGHT);
		int idx = robots[ii].x + robots[ii].y * WIDTH;
		++grid[idx];
	}
	return grid;
}

void single_step(int* grid, struct robot robots[], size_t robots_sz) {
	for (size_t ii = 0; ii < robots_sz; ++ii) {
		int idx = robots[ii].x + robots[ii].y * WIDTH;
		assert(grid[idx] >= 0);
		--grid[idx];
		robots[ii].x = (robots[ii].x + robots[ii].vx) % WIDTH;
		robots[ii].y = (robots[ii].y + robots[ii].vy) % HEIGHT;
		idx = robots[ii].x + robots[ii].y * WIDTH;
		++grid[idx];
	}
}

bool check_nonoverlap(int* grid) {
	for (int ii = 0; ii < WIDTH * HEIGHT; ++ii)
		if (grid[ii] > 1)
			return false;
	return true;
}

void print_grid(int* grid) {
	for (int row = 0; row < HEIGHT; ++row) {
		for (int col = 0; col < WIDTH; ++col) {
			printf("%c", grid[col + row * WIDTH] ? '0' + grid[col + row * WIDTH] : '.');
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct robot robots[MAX_ROBOTS];
	size_t robots_sz = 0;

	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		const char* l = line;
		read_signed_int(&l, &robots[robots_sz].x);
		read_signed_int(&l, &robots[robots_sz].y);
		read_signed_int(&l, &robots[robots_sz].vx);
		read_signed_int(&l, &robots[robots_sz].vy);
		// make sure 0 <= vx < WIDTH

		robots[robots_sz].vx = ((robots[robots_sz].vx % WIDTH) + WIDTH) % WIDTH;
		robots[robots_sz].vy = ((robots[robots_sz].vy % HEIGHT) + HEIGHT) % HEIGHT;
		// printf("p=%d,%d v=%d,%d\n",
		//       robots[robots_sz].x, robots[robots_sz].y, robots[robots_sz].vx, robots[robots_sz].vy);
		++robots_sz;
	}
	free(line);

	int* grid = robots_to_grid(robots, robots_sz);

	// move robots 1 step, count left and right.
	// assumption: No overlapping robos when Xmas tree occurs...
	for (int steps_taken = 0; steps_taken < WIDTH * HEIGHT; ++steps_taken) {
		single_step(grid, robots, robots_sz);
		if (check_nonoverlap(grid)) {
			printf("Steps taken: %d\n", steps_taken + 1);
			print_grid(grid);
		}
	}
	free(grid);
	return 0;
}
