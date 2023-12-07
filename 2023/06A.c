#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
	int prod = 1;

	int times[MAX_RACES];
	int dists[MAX_RACES];

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin); // times
	int nr_races = 0;
	char* l = line;
	while (read_int(&l, times + nr_races))
		++nr_races;
	getline(&line, &len, stdin); // times
	l = line;
	for (int ii = 0; ii < nr_races; ++ii)
		read_int(&l, dists + ii);
	free(line);

	for (int ii = 0; ii < nr_races; ++ii) {
		double ttot = times[ii];
		double d = dists[ii];
		// printf("Time: %6.0f; dist: %6.0f\n", ttot, d);
		double t0 = (ttot - sqrt(ttot * ttot - 4.0 * d)) / 2.0;
		double t1 = (ttot + sqrt(ttot * ttot - 4.0 * d)) / 2.0;
		int tmin = floor(t0) + 1;
		int tmax = ceil(t1) - 1;
		// printf("tmin = %d; tmax = %d; ways: %d\n", tmin, tmax, tmax - tmin + 1);
		prod *= (tmax - tmin + 1);
	}
	printf("%d\n", prod);

	return 0;
}
