#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_ELVES 512

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int cmp_reverse(const void* pa, const void* pb) {
	return *(int *)pb - *(int *)pa;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum[MAX_ELVES] = {0};
	int sum_sz = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line))
			++sum_sz;
		else
			sum[sum_sz] += read_int(&l);
	}
	++sum_sz;
	free(line);

	qsort(sum, sum_sz, sizeof(int), cmp_reverse);
	printf("%d\n", sum[0] + sum[1] + sum[2]);
	return 0;
}
