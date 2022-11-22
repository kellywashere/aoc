#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

int main(int argc, char* argv[]) {
	int n[100];
	int nn = 0;

	n[nn++] = 0; // outlet
	while (scanf("%d", &n[nn]) == 1)
		++nn;
	qsort(n, nn, sizeof(int), cmp);
	n[nn] = n[nn - 1] + 3;
	++nn;

	uint64_t nr_ways[100] = {0}; // nr_ways[n]: how many ways can we reach n[idx]?
	nr_ways[0] = 1; // 1 way to write: (0)
	for (int ii = 1; ii < nn; ++ii) {
		// sweep over last nr before n[ii]
		for (int d = 1; ii - d >= 0 && n[ii] - n[ii - d] <= 3; ++d) {
			nr_ways[ii] += nr_ways[ii - d];
		}
	}
	printf("%" PRIu64 "\n", nr_ways[nn - 1]);

	return 0;
}
