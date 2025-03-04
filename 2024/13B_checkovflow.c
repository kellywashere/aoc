#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

#define MAX_INT64 0xFFFFFFFFFFFFFFFFULL

struct machine {
	int xa;
	int ya;
	int xb;
	int yb;
	int xp;
	int yp;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_int(const char** pLine, int* x) {
	int num = 0;
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

bool read_nonempty_line(char** lineptr, size_t* len) {
	while (getline(lineptr, len, stdin) != -1) {
		if (!empty_line(*lineptr))
			return true;
	}
	return false;
}

bool read_machine(struct machine* m) {
	char *line = NULL;
	size_t len = 0;

	if (!read_nonempty_line(&line, &len))
		return false;
	const char *l = line;
	read_int(&l, &m->xa);
	read_int(&l, &m->ya);

	if (!read_nonempty_line(&line, &len)) {
		free(line);
		return false;
	}
	l = line;
	read_int(&l, &m->xb);
	read_int(&l, &m->yb);

	if (!read_nonempty_line(&line, &len)) {
		free(line);
		return false;
	}
	l = line;
	read_int(&l, &m->xp);
	read_int(&l, &m->yp);

	free(line);
	return true;
}

bool add_check_overflow(uint64_t a, uint64_t b, uint64_t* sum) {
	if (a > MAX_INT64 - b) return false;
	*sum = a + b;
	return true;
}

bool mul_check_overflow(uint64_t a, uint64_t b, uint64_t* prod) {
	*prod = 0;
	uint64_t sum = 0;
	while (b) {
		uint64_t b0 = b & 1;
		if (b0) {
			// *prod += a
			if (!add_check_overflow(a, *prod, &sum))
				return false;
			*prod = sum;
		}
		if ((a >> 63) == 1)
			return false;
		a <<= 1;
		b >>= 1;
	}
	return true;
}

bool calc_ab(struct machine* m, uint64_t* a, uint64_t* b) {
	int den = m->xa * m->yb - m->ya * m->xb;
	if (den == 0) return false;

	const uint64_t offset = 10000000000000ULL;
	uint64_t xp = offset + m->xp;
	uint64_t yp = offset + m->yp;

	// numa = xp * m->yb - yp * m->xb;
	uint64_t numa_p1, numa_p2, numa;
	if (!mul_check_overflow(xp, m->yb, &numa_p1))
		printf("Overflow when multplying %" PRIu64 " x %d\n", xp, m->yb);
	assert(numa_p1 == xp * m->yb);
	if (!mul_check_overflow(yp, m->xb, &numa_p2))
		printf("Overflow when multplying %" PRIu64 " x %d\n", yp, m->xb);
	assert(numa_p2 == yp * m->xb);
	if (numa_p1 > numa_p2) {
		numa = numa_p1 - numa_p2;
		if (den < 0) return false;
		if (numa % den != 0) return false;
		*a = numa / den;
	}
	else {
		numa = numa_p2 - numa_p1;
		if (den > 0) return false;
		if (numa % (-den) != 0) return false;
		*a = numa / (-den);
	}

	// numb = yp * m->xa - xp * m->ya;
	uint64_t numb_p1, numb_p2, numb;
	if (!mul_check_overflow(yp, m->xa, &numb_p1))
		printf("Overflow when multplying %" PRIu64 " x %d\n", yp, m->xa);
	assert(numb_p1 == yp * m->xa);
	if (!mul_check_overflow(xp, m->ya, &numb_p2))
		printf("Overflow when multplying %" PRIu64 " x %d\n", xp, m->ya);
	assert(numb_p2 == xp * m->ya);
	if (numb_p1 > numb_p2) {
		numb = numb_p1 - numb_p2;
		if (den < 0) return false;
		if (numb % den != 0) return false;
		*b = numb / den;
	}
	else {
		numb = numb_p2 - numb_p1;
		if (den > 0) return false;
		if (numb % (-den) != 0) return false;
		*b = numb / (-den);
	}
	return true;
}

int main(int argc, char* argv[]) {
	struct machine m;

	uint64_t tot = 0;
	while (read_machine(&m)) {
		/*
		printf("Button A: X+%d, Y+%d\n", m.xa, m.ya);
		printf("Button B: X+%d, Y+%d\n", m.xb, m.yb);
		printf("Prize: X=%d, Y=%d\n\n", m.xp, m.yp);
		*/
		uint64_t a, b;
		if (calc_ab(&m, &a, &b))
			tot += a*3 + b;
	}

	printf("%" PRIu64 "\n", tot);

	return 0;
}
