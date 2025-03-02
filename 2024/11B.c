#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

#define MAP_ROWS 0x100000 /* MAP_ROWS has to have only 1 bit in binary */

struct lup {
	int      nr_rows;
	int      nr_cols;
	uint64_t lup[];
};

bool read_uint64(const char** pLine, uint64_t* x) {
	uint64_t num = 0;
	const char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

int count_digits(uint64_t x) {
	if (x == 0) return 1;
	int digs = 0;
	while (x) {
		x /= 10;
		++digs;
	}
	return digs;
}

void split_number(uint64_t x, int digits_to_cut, uint64_t* y1, uint64_t* y2) {
	uint64_t p10 = 1;
	while (digits_to_cut > 0) {
		p10 *= 10;
		--digits_to_cut;
	}
	*y1 = x / p10;
	*y2 = x % p10;
}

// recursive
uint64_t count_stones(uint64_t x, int steps_left, struct lup* lup) {
	if (steps_left == 0) {
		//printf("  %" PRIu64 "\n", x);
		return 1;
	}
	--steps_left;
	int hash = x & (MAP_ROWS - 1);
	int idx_key = hash * lup->nr_cols;
	assert(steps_left + 1 < lup->nr_cols);
	int idx_val = idx_key + steps_left + 1;
	if (lup->lup[idx_key] == x && lup->lup[idx_val] > 0)
		return lup->lup[idx_val];

	uint64_t ret;
	if (x == 0) {
		ret = count_stones(1, steps_left, lup);
	}
	else {
		int nr_digits = count_digits(x);
		if (nr_digits % 2 == 0) {
			uint64_t y1, y2;
			split_number(x, nr_digits / 2, &y1, &y2);
			ret = count_stones(y1, steps_left, lup) + count_stones(y2, steps_left, lup);
		}
		else
			ret = count_stones(x * 2024, steps_left, lup);
	}
	// update LUT
	if (lup->lup[idx_key] == 0) {
		lup->lup[idx_key] = x;
		lup->lup[idx_val] = ret;
	}
	else if (lup->lup[idx_key] == x) {
		lup->lup[idx_val] = ret;
	}
	else {
		//printf("Collision: 0x%" PRIx64 " in map, we have x = 0x%" PRIx64 "\n", lup->lup[idx_key], x); // collision :(
	}
	return ret;
}

int main(int argc, char* argv[]) {
	int steps = 75;
	if (argc > 1) steps = atoi(argv[1]);

	// allocate LUT memory, set all entries to 0
	int nr_rows = MAP_ROWS;
	int nr_cols = steps + 1; // col 0 is for collision detection
	struct lup* lup = malloc(sizeof(struct lup) + (nr_rows * nr_cols) * sizeof(uint64_t));
	if (lup == NULL) {
		fprintf(stderr, "Could not get memory for lup\n");
		return 1;
	}
	lup->nr_rows = nr_rows;	
	lup->nr_cols = nr_cols;
	for (int ii = 0; ii < lup->nr_rows * lup->nr_cols; ++ii)
		lup->lup[ii] = 0;

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	const char* l = line; // copy because we cannot change line (free later)
	uint64_t x;
	uint64_t tot = 0;
	while (read_uint64(&l,  &x)) {
		tot += count_stones(x, steps, lup);
	}

	printf("%" PRIu64 "\n", tot);

	free(lup);
	free(line);
	return 0;
}
