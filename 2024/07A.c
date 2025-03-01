#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#define MAX_NR_OPERANDS 32

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

// recursive
bool can_equal(uint64_t target, uint64_t nrs[], size_t nrs_sz) {
	uint64_t lastnr =  nrs[nrs_sz - 1];

	// base case
	if (nrs_sz == 1)
		return target == lastnr;

	// try * operator
	if (target % lastnr == 0) {
		if (can_equal(target / lastnr, nrs, nrs_sz - 1))
			return true;
	}
	// try + operator
	if (target - lastnr >= 0) {
		if (can_equal(target - lastnr, nrs, nrs_sz - 1))
			return true;
	}
	return false;
}

int main(int argc, char* argv[]) {
	uint64_t target;
	uint64_t nrs[MAX_NR_OPERANDS];
	size_t nrs_sz = 0;

	uint64_t tot = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		const char* l = line;
		if (!read_uint64(&l, &target)) continue;
		nrs_sz = 0;
		while (read_uint64(&l, nrs + nrs_sz))
			++nrs_sz;
		bool works = can_equal(target, nrs, nrs_sz);
		tot += works ? target : 0;
	}

	printf("%" PRIu64 "\n", tot);

	free(line);
	return 0;
}
