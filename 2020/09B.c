#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#define NR_SUM_ITEMS 25

/*
int cmp(const void* a, const void* b) {
	uint64_t xa = *(uint64_t*)a;
	uint64_t xb = *(uint64_t*)b;
	return xa > xb ? 1 : xa == xb ? 0 : -1;
}
*/

uint64_t read_uint64(char** pLine) {
	uint64_t num = 0;
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

bool is_sum(uint64_t* vals, int idx) {
	for (int i1 = idx - NR_SUM_ITEMS; i1 < idx - 1; ++i1)
		for (int i2 = i1 + 1; i2 < idx; ++i2)
			if (vals[i1] + vals[i2] == vals[idx])
				return true;
	return false;
}

uint64_t find_first_invalid(uint64_t* vals, int vals_sz) {
	for (int ii = NR_SUM_ITEMS; ii < vals_sz; ++ii) {
		if (!is_sum(vals, ii))
			return vals[ii];
	}
	return 0;
}

uint64_t find_sum_seq(uint64_t* vals, int vals_sz, uint64_t target) {
	int ii1 = 0;
	int ii2 = 1;
	uint64_t s = vals[ii1] + vals[ii2];
	while (s != target && ii2 < vals_sz) {
		if (s < target) {
			++ii2;
			s += vals[ii2];
		}
		else {
			s -= vals[ii1];
			++ii1;
		}
	}
	uint64_t smallest = target;
	uint64_t largest = 0;
	for (int ii = ii1; ii <= ii2; ++ii) {
		smallest = vals[ii] < smallest ? vals[ii] : smallest;
		largest = vals[ii] > largest ? vals[ii] : largest;
	}
	return (smallest + largest);
}

int main(int argc, char* argv[]) {
	uint64_t vals[1024];
	int nr_vals = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		vals[nr_vals++] = read_uint64(&l);
	}
	free(line);
	uint64_t s = find_first_invalid(vals, nr_vals);
	s = find_sum_seq(vals, nr_vals, s);
	printf("%" PRIu64 "\n", s);

	return 0;
}
