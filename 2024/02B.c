#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAXDEPTH 128

bool read_int(char** pLine, int* x) {
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
	*x = num;
	*pLine = line;
	return true;
}

#define MAX_LIST_LEN 64

bool is_list_safe(int* list, int len, int skipidx) {
	// special cases
	if (len <= 2) return true;
	int dir = 0; // unknown if inc or dec
	int prev;
	bool prev_is_valid = false;

	for (int ii = 0; ii < len; ++ii) {
		if (ii == skipidx) continue;
		if (dir == 0 && prev_is_valid) {
			dir = list[ii] - prev > 0 ? 1 : -1;
		}

		if (prev_is_valid) {
			int diff = (list[ii] - prev) * dir;
			if (diff < 1 || diff > 3)
				return false;
		}

		prev = list[ii];
		prev_is_valid = true;
	}
	return true;
}

int main(int argc, char* argv[]) {
	int list[MAX_LIST_LEN] = {0};

	int count_safe = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int x;
		int listlen = 0;
		while (read_int(&l, &x) && listlen < MAX_LIST_LEN)
			list[listlen++] = x;
		
		bool safe = is_list_safe(list, listlen, 0);
		for (int rmidx = 1; !safe && rmidx < listlen; ++rmidx) {
			safe = is_list_safe(list, listlen, rmidx);
		}
		count_safe += safe ? 1 : 0;
	}

	printf("%d\n", count_safe);
	free(line);
	return 0;
}
