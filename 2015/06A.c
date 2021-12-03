#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define GRIDSIZE 1000

enum command {
	ON,
	OFF,
	TOGGLE
};

void parse_coordinate(const char* line, int* idx, int* x, int* y) {
	char* endptr;
	*x = (int)strtol(line + *idx, &endptr, 10);
	if (*endptr != ',')
		fprintf(stderr, "Did not see , in coordinates: %s\n", line);
	*y = (int)strtol(endptr + 1, &endptr, 10);
	*idx = endptr - line;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	int x, y;
	bool* grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(bool));
	while (getline(&line, &len, stdin) != -1) {
		int idx = 0;
		int x0, y0, x1, y1;
		enum command cmd = ON;
		// parse command
		if (!strncmp(line, "turn on", 7)) {
			idx = 8;
			cmd = ON;
		}
		else if (!strncmp(line, "turn off", 8)) {
			idx = 9;
			cmd = OFF;
		}
		else if (!strncmp(line, "toggle", 6)) {
			idx = 7;
			cmd = TOGGLE;
		}
		else {
			fprintf(stderr, "unknown cmd %s\n", line);
			return 1;
		}
		parse_coordinate(line, &idx, &x0, &y0);
		// find next coordinate start
		while (line[idx] < '0' || line[idx] > '9')
			++idx;
		parse_coordinate(line, &idx, &x1, &y1);
		for (x = x0; x <= x1; ++x) {
			for (y = y0; y <= y1; ++y) {
				int grididx = x + y * GRIDSIZE;
				grid[grididx] = cmd == ON ? true : (cmd == OFF ? false : !grid[grididx]);
			}
		}
	}
	free(line);

	for (x = 0; x < GRIDSIZE; ++x) {
		for (y = 0; y < GRIDSIZE; ++y) {
			int grididx = x + y * GRIDSIZE;
			if (grid[grididx])
				++count;
		}
	}
	
	printf("%d\n", count);
	return 0;
}
