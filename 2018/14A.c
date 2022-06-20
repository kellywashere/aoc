#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int steps = 430971; // puzzle input
	if (argc > 1)
		steps = atoi(argv[1]);
	int totsteps = steps + 10 + 2; // +2 margin to prevent additional checking
	int* nrs = malloc(totsteps * sizeof(int));
	int  n = 0; // nr of nrs

	nrs[n++] = 3;
	nrs[n++] = 7;

	int elfidx[2] = {0, 1};

	while (n < totsteps) {
		int s = nrs[elfidx[0]] + nrs[elfidx[1]];
		if (s >= 10)
			nrs[n++] = s / 10;
		nrs[n++] = s % 10;
		elfidx[0] = (elfidx[0] + 1 + nrs[elfidx[0]]) % n;
		elfidx[1] = (elfidx[1] + 1 + nrs[elfidx[1]]) % n;
	}
	for (int ii = 0; ii < 10; ++ii)
		printf("%d", nrs[steps + ii]);
	printf("\n");

	free(nrs);
	return 0;
}
