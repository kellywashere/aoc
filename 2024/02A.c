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
		bool safe = true;
		if (listlen > 1) {
			int dir = list[1] > list[0] ? 1 : -1;
			for (int ii = 1; ii < listlen; ++ii) {
				int diff = (list[ii] - list[ii - 1]) * dir;
				safe = safe && (1 <= diff && diff <= 3);
			}
		}
		count_safe += safe ? 1 : 0;
	}

	printf("%d\n", count_safe);
	free(line);
	return 0;
}
