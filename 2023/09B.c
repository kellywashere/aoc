#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <assert.h>

#define MAX_LISTLEN 24

bool read_int(char** pLine, int* x) {
	// skips all chars until a digit is found, then reads nr
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	if (isdigit(*line) && line > *pLine && *(line - 1) == '-')
		isneg = true;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	*x = isneg ? (-num) : num;
	return true;
}

void print_list(int list[], int listlen) {
	for (int ii = 0; ii < listlen - 1; ++ii)
		printf("%d, ", list[ii]);
	printf("%d\n", list[listlen - 1]);
}

int prev_number(int list[], int listlen) {
	assert(listlen >= 1);
	// check if all the same: return that nr
	bool same = true;
	for (int ii = 1; ii < listlen; ++ii)
		same = same && (list[ii] == list[ii - 1]);
	if (same)
		return list[0];

	// create diff list
	int difflist[MAX_LISTLEN];
	for (int ii = 1; ii < listlen; ++ii)
		difflist[ii - 1] = list[ii] - list[ii - 1];
	int n = prev_number(difflist, listlen - 1); // Recursion!
	return list[0] - n;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	int sum = 0;

	while (getline(&line, &len, stdin) != -1) {
		int x;
		int list[MAX_LISTLEN];
		int listlen = 0;
		char* l = line;
		while (read_int(&l, &x)) {
			list[listlen++] = x;
		}
		sum += prev_number(list, listlen);
	}
	free(line);
	printf("%d\n", sum);

	return 0;
}
