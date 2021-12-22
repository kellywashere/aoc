#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

void read_coords(char** line, int* w1, int* w2) {
	*w1 = strtol(*line, line, 10);
	*line += 2;
	*w2 = strtol(*line, line, 10);
}

void process_line(char* line, bool* grid) {
	int x1, x2, y1, y2, z1, z2;
	int turnon = false;
	if (line[1] == 'n') { // on
		turnon = true;
		line += 5;
	}
	else
		line += 6;
	read_coords(&line, &x1, &x2);
	line += 3;
	read_coords(&line, &y1, &y2);
	line += 3;
	read_coords(&line, &z1, &z2);
	if ((x1 < -50 && x2 < -50) || (x1 > 50 && x2 > 50))
		return;
	if ((y1 < -50 && y2 < -50) || (y1 > 50 && y2 > 50))
		return;
	if ((z1 < -50 && z2 < -50) || (z1 > 50 && z2 > 50))
		return;
	// printf("%d..%d, %d..%d, %d..%d\n", x1, x2, y1, y2, z1, z2);
	int x,y,z;
	for (x = x1; x <= x2; ++x) {
		for (y = y1; y <= y2; ++y) {
			for (z = z1; z <= z2; ++z) {
				grid[(x + 50)*101*101 + (y + 50)*101 + (z + 50)] = turnon;
			}
		}
	}
}

int count_lights(bool* grid) {
	int count = 0;
	int ii = 0;
	for (ii = 0; ii < 101 * 101 * 101; ++ii)
		count += grid[ii] ? 1 : 0;
	return count;
}

int main(int argc, char* argv[]) {
	bool grid[101 * 101 * 101] = {false};
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		process_line(line, grid);
	}
	printf("%d\n", count_lights(grid));
	free(line);
	return 0;
}
