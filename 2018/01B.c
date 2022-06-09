#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// NOTE: algo used here does not work if sum of nrs is 0

#define MAX_NRS 1024

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

int main(int argc, char* argv[]) {
	int cumsum[MAX_NRS];
	int nr_n = 0;

	char *line = NULL;
	size_t len = 0;
	int cycle_sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int x = read_int(&l);
		cycle_sum += x;
		cumsum[nr_n] = cycle_sum;
		++nr_n;
	}
	free(line);

	// printf("Cycle sum: %d\n", cycle_sum);

	// sum that appears twice: first on idx ii, then again when
	// hitting jj, possibly going around k full cycles first
	int min_idx = INT_MAX;
	int double_nr = 0;
	for (int ii = 0; ii < nr_n; ++ii) {
		for (int steps = 1; steps < nr_n; ++steps) {
			int jj = (ii + steps) % nr_n;
			// condition: going from [ii] to [k*nr_n + jj], sum is increased by 0
			int s = jj > ii ? cumsum[jj] - cumsum[ii] : cumsum[jj] - cumsum[ii] + cycle_sum;
			// s + k*cycle_sum = 0
			if (s % cycle_sum == 0) {
				int k = -s / cycle_sum;
				if (k >= 0) {
					int jj_tot = ii + steps + k*nr_n;
					// printf("c[%d] = %d, c[%d] = %d (s = %d):  ", ii, cumsum[ii], jj, cumsum[jj], s);
					// printf("c[%d]=%d, c[%d]=%d\n", ii, cumsum[ii], jj_tot, cumsum[ii] + s + k*cycle_sum);
					if (jj_tot < min_idx) {
						min_idx = jj_tot;
						double_nr = cumsum[ii];
					}
				}
			}
		}
	}

	printf("%d\n", double_nr);

	return 0;
}
