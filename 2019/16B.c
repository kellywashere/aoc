#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define ABS(a) (((a)<0)?-(a):(a))
#define REPEAT 10000

int pattern[] = {0, 1, 0, -1};

// period of correlation "wave" is 4 for first row, 8 for 2nd, 12 for 3rd etc
// T = 4 * k where k is index of output nr we are calculating
// We are interestes in numbers starting from index offset, which is quite large compared
// to total len L. In fact: offset/L > 0.9
// this means that the correlating wave for the numbers we are interested in looks like:
// k-1 zeros, followed by (L - (k-1)) ones (if we want to calc nr at pos k)
// Xk = sum(kth input ... Lth input) % 10   !!!
// We can totally ignore first k-1 nrs

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	// count digits
	int nr_digits = 0;
	for ( ; isdigit(line[nr_digits]); ++nr_digits)
		;
	int L = nr_digits * REPEAT;
	
	// we need to create list of length L - k + 1
	
	// calc offset
	int skip = 0;
	for (int ii = 0; ii < 7; ++ii)
		skip = 10 * skip + line[ii] - '0';

	if (skip < L/2) {
		printf("Algo assumes offset is larger than total len / 2\n");
		exit(1);
	}

	// we need a list to remember values at postions k .. L only
	int listlen = L - skip;
	int* curlist = malloc(listlen * sizeof(int));
	for (int ii = 0; ii < listlen; ++ii) {
		int idx = (skip + ii) % nr_digits;
		curlist[ii] = line[idx] - '0';
	}
	// we need to run a special version of fft on curlist:
	// nextlist[k] = sum(curlist[k]..curlist[listlen]) % 10

	int* nextlist = malloc(listlen * sizeof(int));
	for (int step = 0; step < 100; ++step) {
		int sum = 0;
		for (int ii = listlen - 1; ii >= 0; --ii) {
			sum = (sum + curlist[ii]) % 10;
			nextlist[ii] = sum;
		}
		int *tmp = curlist;
		curlist = nextlist;
		nextlist = tmp;
	}

	// print first 8 digits
	for (int ii = 0; ii < 8; ++ii)
		printf("%d", curlist[ii]);
	printf("\n");

	free(nextlist);
	free(curlist);
	free(line);
	return 0;
}
