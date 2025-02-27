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
	int idx1 = 0;
	int idx2 = 0;
	while (idx1 < len) {
		int n = list1[idx1];
		int count1 = 0;
		while (idx1 < len && list1[idx1] == n) {
			++count1;
			++idx1;
		}
		while (idx2 < len && list2[idx2] < n)
			++idx2;
		int count2 = 0;
		while (idx2 < len && list2[idx2] == n) {
			++count2;
			++idx2;
		}
		tot += n * count1 * count2;
	}
	printf("%" PRId64 "\n", tot);

	return 0;
}
