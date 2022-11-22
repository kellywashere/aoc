#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

bool find_sum2(int n[], int nn, int target, int* i1, int* i2) {
	// list n[] should already be sorted!
	*i1 = 0;
	*i2 = nn - 1;
	while (*i2 >= *i1 && n[*i1] + n[*i2] != target) {
		if (n[*i1] + n[*i2] < target)
			++(*i1);
		else
			--(*i2);
	}
	return (n[*i1] + n[*i2] == target);
}

int main(int argc, char* argv[]) {
	int n[1000];
	int nn = 0;

	while (scanf("%d", &n[nn]) == 1)
		++nn;
	qsort(n, nn, sizeof(int), cmp);

	int i1, i2;
	int i3 = nn - 1;
	int prod = 0;
	while (!prod && i3 >= 2 && n[i3] + n[i3 - 1] + n[i3 - 2] >= 2020) {
		if (find_sum2(n, i3, 2020 - n[i3], &i1, &i2))
			prod = n[i1] * n[i2] * n[i3];
		--i3;
	}
	printf("%d\n", prod);

	return 0;
}
