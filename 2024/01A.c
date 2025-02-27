#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>

int cmp_int(const void* pa, const void* pb) {
	return *(int*)pa - *(int*)pb;
}

int main(int argc, char* argv[]) {
	int list1[1024];
	int list2[1024];
	int len = 0;
	while (scanf("%d %d", list1 + len, list2 + len) == 2)
		++len;

	qsort(list1, len, sizeof(int), cmp_int);
	qsort(list2, len, sizeof(int), cmp_int);

	int64_t tot = 0;
	for (int ii = 0; ii < len; ++ii) {
		int diff = list1[ii] - list2[ii];
		tot += diff < 0 ? (-diff) : diff;
	}
	printf("%" PRId64 "\n", tot);

	return 0;
}
