#include <stdio.h>
#include <stdlib.h>

int cmp(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

int main(int argc, char* argv[]) {
	int s[3];
	int total = 0;

	while (scanf("%dx%dx%d", &s[0], &s[1], &s[2]) != EOF) {
		qsort(s, 3, sizeof(int), cmp);
		total += 3*s[0]*s[1] + 2*(s[1]*s[2] + s[0]*s[2]);
	}
	printf("%d\n", total);
	
	return 0;
}
