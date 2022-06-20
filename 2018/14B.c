#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXSTEPS 30000000

bool find_pattern(int* nrs, int n, int seq) {
	while (seq) {
		int s = seq % 10;
		seq /= 10;
		if (nrs[--n] != s)
			return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	int sequence = 430971; // seq to look for
	if (argc > 1)
		sequence = atoi(argv[1]);
	int  capacity = 4096; // initial cacity of nrs
	int* nrs = malloc(capacity * sizeof(int));
	int  n = 0; // nr of nrs

	nrs[n++] = 3;
	nrs[n++] = 7;

	int elfidx[2] = {0, 1};

	int lastseqdig = sequence % 10; // aids slightly faster search

	int found_n = 0;
	while (found_n == 0 && n < MAXSTEPS) {
		if (n + 2 > capacity) {
			capacity *= 2;
			nrs = realloc(nrs, capacity * sizeof(int));
		}
		int s = nrs[elfidx[0]] + nrs[elfidx[1]];
		int s1 = s / 10;
		int s2 = s % 10;
		if (s1) {
			nrs[n++] = s1;
			if (s1 == lastseqdig && find_pattern(nrs, n, sequence))
				found_n = n;
		}
		nrs[n++] = s2;
		if (found_n == 0 && s2 == lastseqdig && find_pattern(nrs, n, sequence))
			found_n = n;
		elfidx[0] = (elfidx[0] + 1 + nrs[elfidx[0]]) % n;
		elfidx[1] = (elfidx[1] + 1 + nrs[elfidx[1]]) % n;
	}
	// subtract seq len from found idx
	int s = sequence;
	while (s) {
		s /= 10;
		--found_n;
	}
	printf("%d\n", found_n);

	free(nrs);
	return 0;
}
