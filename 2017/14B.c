#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void reverse_list(int* list, int pos, int len) {
	for (int ii = 0; ii < len / 2; ++ii) {
		int idx1 = (pos + ii) & 255;
		int idx2 = (pos + len - ii - 1) & 255;
		int v = list[idx1];
		list[idx1] = list[idx2];
		list[idx2] = v;
	}
}

int suffix[] = {17, 31, 73, 47, 23};

void knot_hash(const char* str, int* hash) {
	int sparse[256];
	char input[1024];
	// init sparse hash data
	for (int ii = 0; ii < 256; ++ii)
		sparse[ii] = ii;
	int pos = 0;
	int skipsize = 0;
	strncpy(input, str, 1023);
	// add 17, 31, 73, 47, 23
	int inlen = strlen(input);
	for (int ii = 0; ii < 5; ++ii)
		input[inlen++] = suffix[ii];
	input[inlen] = '\0';
	for (int round = 0; round < 64; ++round) {
		for (int ii = 0; input[ii] != '\0'; ++ii) {
			reverse_list(sparse, pos, input[ii]);
			pos = (pos + input[ii] + skipsize) & 255;
			skipsize = (skipsize + 1) & 255;
		}
	}
	// sparse to dense:
	for (int didx = 0; didx < 16; ++didx) {
		hash[didx] = 0;
		for (int ii = 0; ii < 16; ++ii)
			hash[didx] ^= sparse[16 * didx + ii];
	}
}

void flood_fill(int* g, int x, int y, int v) {
	int idx = y * 128 + x;
	g[idx] = v;
	if (x > 0 && g[idx - 1] == -1)
		flood_fill(g, x - 1, y, v);
	if (x < 127 && g[idx + 1] == -1)
		flood_fill(g, x + 1, y, v);
	if (y > 0 && g[idx - 128] == -1)
		flood_fill(g, x, y - 1, v);
	if (y < 127 && g[idx + 128] == -1)
		flood_fill(g, x, y + 1, v);
}

int count_regions(bool* grid) {
	// create helper grid
	int helper[128*128];
	for (int idx = 0; idx < 128*128; ++idx)
		helper[idx] = grid[idx] ? -1 : 0; // used: -1
	int regionnr = 1;
	for (int y = 0; y < 128; ++y) {
		for (int x = 0; x < 128; ++x) {
			int idx = y * 128 + x;
			if (helper[idx] == -1)
				flood_fill(helper, x, y, regionnr++);
		}
	}
	return regionnr - 1;
}

int main(int argc, char* argv[]) {
	bool grid[128*128];

	char* input = "jzgqcdpd";
	if (argc > 1)
		input = argv[1];

	for (int row = 0; row < 128; ++row) {
		char buf[80];
		int dense[16];
		sprintf(buf, "%s-%d", input, row);

		knot_hash(buf, dense);
		for (int ii = 0; ii < 16; ++ii) {
			for (int b = 7; b >= 0; --b) {
				grid[row * 128 + 8 * ii + (7 - b)] = (dense[ii] >> b) & 1;
			}
		}
	}
	printf("%d\n", count_regions(grid));

	return 0;
}
