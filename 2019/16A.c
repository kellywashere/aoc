#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define ABS(a) (((a)<0)?-(a):(a))

int pattern[] = {0, 1, 0, -1};

void fft_step(int* in, int* out, int len) {
	for (int out_idx = 0; out_idx < len; ++out_idx) {
		int rep = out_idx + 1; // repetitions of pattern entry
		int patcnt = 1; // skip 1 at start
		int pat_idx = 0;
		int sum = 0;
		for (int in_idx = 0; in_idx < len; ++in_idx) {
			if (patcnt == rep) {
				patcnt = 0;
				pat_idx = (pat_idx + 1) % 4;
			}
			sum += pattern[pat_idx] * in[in_idx];
			++patcnt;
		}
		out[out_idx] = ABS(sum) % 10;
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	// count digits
	int nr_digits = 0;
	for ( ; isdigit(line[nr_digits]); ++nr_digits)
		;

	int* curlist  = malloc(nr_digits * sizeof(int));
	for (int ii = 0; ii < nr_digits; ++ii)
		curlist[ii] = line[ii] - '0';

	int* nextlist = malloc(nr_digits * sizeof(int));
	for (int step = 0; step < 100; ++step) {
		fft_step(curlist, nextlist, nr_digits);
		int *tmp = curlist;
		curlist = nextlist;
		nextlist = tmp;
	}

	// print first 8 digits
	for (int ii = 0; ii < 8; ++ii)
		printf("%d", curlist[ii]);
	printf("\n");

	free(curlist);
	free(nextlist);
	free(line);
	return 0;
}
