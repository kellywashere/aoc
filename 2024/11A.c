#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

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
uint64_t count_stones(uint64_t x, int steps_left) {
	if (steps_left == 0) {
		//printf("  %" PRIu64 "\n", x);
		return 1;
	}
	--steps_left;

	if (x == 0)
		return count_stones(1, steps_left);
	int nr_digits = count_digits(x);
	if (nr_digits % 2 == 0) {
		uint64_t y1, y2;
		split_number(x, nr_digits / 2, &y1, &y2);
		return count_stones(y1, steps_left) + count_stones(y2, steps_left);
	}
	return count_stones(x * 2024, steps_left);
}

int main(int argc, char* argv[]) {
	int steps = 25;
	if (argc > 1) steps = atoi(argv[1]);

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	const char* l = line; // copy because we cannot change line (free later)
	uint64_t x;
	uint64_t tot = 0;
	while (read_uint64(&l,  &x)) {
		tot += count_stones(x, steps);
	}

	printf("%" PRIu64 "\n", tot);

	free(line);
	return 0;
}
