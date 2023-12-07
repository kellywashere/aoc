#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <math.h>

typedef uint64_t u64;

bool read_int(char** pLine, int* x) {
	// skips all chars until a digit is found, then reads nr
	int num = 0;
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
	*x = num;
	return true;
}

#define MAX_RACES 16

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin); // times
	char* l = line;
	u64 time = 0;
	while (*l) {
		if (isdigit(*l))
			time = time * 10 + *l - '0';
		++l;
	}
	getline(&line, &len, stdin); // times
	l = line;
	u64 dist = 0;
	while (*l) {
		if (isdigit(*l))
			dist = dist * 10 + *l - '0';
		++l;
	}
	free(line);
	printf("Time: %"PRIu64"\n", time);
	printf("Dist: %"PRIu64"\n", dist);

	double ttot = time;
	double d = dist;
	// printf("Time: %6.0f; dist: %6.0f\n", ttot, d);
	double t0 = (ttot - sqrt(ttot * ttot - 4.0 * d)) / 2.0;
	double t1 = (ttot + sqrt(ttot * ttot - 4.0 * d)) / 2.0;
	u64 tmin = floor(t0) + 1;
	u64 tmax = ceil(t1) - 1;
	printf("tmin = %"PRIu64"; tmax = %"PRIu64"; ways: %"PRIu64"\n", tmin, tmax, tmax - tmin + 1);

	return 0;
}
