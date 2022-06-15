#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRID_WIDTH 300

int calc_power_level(int x, int y, int serial) {
	// NOTE: x and y are ZERO based here, but 1-based in puzzle
	int rid = x + 1 + 10;
	int p = (rid * (y + 1) + serial) * rid;
	return (p / 100) % 10 - 5;
}

int main(int argc, char* argv[]) {
	int serial = 8199;
	if (argc > 1)
		serial = atoi(argv[1]);
	// fill grid
	int grid[GRID_WIDTH * GRID_WIDTH];
	for (int y = 0; y < GRID_WIDTH; ++y)
		for (int x = 0; x < GRID_WIDTH; ++x)
			grid[y * GRID_WIDTH + x] = calc_power_level(x, y, serial);
	// row cumsum
	int rowcumsum[GRID_WIDTH * GRID_WIDTH];
	for (int y = 0; y < GRID_WIDTH; ++y) {
		int cumsum = 0;
		for (int x = 0; x < GRID_WIDTH; ++x) {
			int idx = y * GRID_WIDTH + x;
			cumsum += grid[idx];
			rowcumsum[idx] = cumsum;
		}
	}

	// find largest sq x sq square
	int maxp = 0;
	int maxx = 0;
	int maxy = 0;
	int maxsq = 0;
	for (int sq = 1; sq <= GRID_WIDTH; ++sq) {
		for (int y = 0; y < GRID_WIDTH - sq + 1; ++y) {
			for (int x = 0; x < GRID_WIDTH - sq + 1; ++x) {
				int tot = 0;
				int idx = y * GRID_WIDTH + x;
				for (int row = 0; row < sq; ++row) {
					tot += rowcumsum[idx + sq - 1] - rowcumsum[idx] + grid[idx];
					idx += GRID_WIDTH;
				}
				if (tot > maxp) {
					maxp = tot;
					maxx = x;
					maxy = y;
					maxsq = sq;
				}
			}
		}
	}
	printf("%d,%d,%d\n", maxx + 1, maxy + 1, maxsq);

	return 0;
}
