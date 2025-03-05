#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

const unsigned int MAX_ROBOTS = 1000;

const unsigned int WIDTH  = 101;
const unsigned int HEIGHT = 103;
const unsigned int STEPS  = 100;

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

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct robot robots[MAX_ROBOTS];
	size_t robots_sz = 0;

	int width = (argc > 1 ? atoi(argv[1]) : WIDTH);
	int height = (argc > 2 ? atoi(argv[2]) : HEIGHT);
	int steps = (argc > 3 ? atoi(argv[3]) : STEPS);

	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		const char* l = line;
		read_signed_int(&l, &robots[robots_sz].x);
		read_signed_int(&l, &robots[robots_sz].y);
		read_signed_int(&l, &robots[robots_sz].vx);
		read_signed_int(&l, &robots[robots_sz].vy);
		//printf("p=%d,%d v=%d,%d\n",
		//      robots[robots_sz].x, robots[robots_sz].y, robots[robots_sz].vx, robots[robots_sz].vy);
		// make sure 0 <= vx < width
		robots[robots_sz].vx = ((robots[robots_sz].vx % width) + width) % width;
		robots[robots_sz].vy = ((robots[robots_sz].vy % height) + height) % height;
		++robots_sz;
	}

	size_t count[4] = {0};
	// move robots, count
	for (size_t ii = 0; ii < robots_sz; ++ii) {
		robots[ii].x = (robots[ii].x + steps * robots[ii].vx ) % width;
		robots[ii].y = (robots[ii].y + steps * robots[ii].vy ) % height;

		printf("Robot at %d,%d\n", robots[ii].x, robots[ii].y);

		if (robots[ii].x != (width - 1)/2 && robots[ii].y != (height - 1)/2) {
			int segment = robots[ii].x > (width - 1) / 2 ? 1 : 0;
			segment += robots[ii].y > (height - 1) / 2 ? 2 : 0;
			++count[segment];
		}
	}
	int prod = count[0] * count[1] * count[2] * count[3];
	printf("%d\n", prod);

	free(line);
	return 0;
}
