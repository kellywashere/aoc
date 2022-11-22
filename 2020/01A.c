#include <stdio.h>
#include <stdlib.h>

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

int main(int argc, char* argv[]) {
	int n[1000];
	int nn = 0;

	while (scanf("%d", &n[nn]) == 1)
		++nn;
	qsort(n, nn, sizeof(int), cmp);

	int i1 = 0;
	int i2 = nn - 1;
	while (i2 >= i1 && n[i1] + n[i2] != 2020) {
		if (n[i1] + n[i2] < 2020)
			++i1;
		else
			--i2;
	}
	if (n[i1] + n[i2] == 2020)
		printf("%d\n", n[i1] * n[i2]);

	return 0;
}
