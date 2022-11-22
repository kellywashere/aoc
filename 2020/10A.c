#include <stdio.h>
#include <stdlib.h>

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

int count_diffs(int* n, int nn, int d) {
	int c = 0;
	for (int ii = 1; ii < nn; ++ii)
		c += n[ii] - n[ii - 1] == d ? 1 : 0;
	return c;
}

int main(int argc, char* argv[]) {
	int n[100];
	int nn = 0;

	n[nn++] = 0; // outlet
	while (scanf("%d", &n[nn]) == 1)
		++nn;
	qsort(n, nn, sizeof(int), cmp);

	int d1 = count_diffs(n, nn, 1);
	int d3 = count_diffs(n, nn, 3) + 1; // +1: count last adapter
	printf("%d\n", d1 * d3);

	return 0;
}
