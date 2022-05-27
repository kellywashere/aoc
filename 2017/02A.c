#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool read_int(char** pLine, int* x) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
	*pLine = line;
	return true;
}

int checksum(char* l) {
	int x = 0;
	read_int(&l, &x);
	int lowest = x;
	int highest = x;
	while (read_int(&l, &x)) {
		lowest = x < lowest ? x : lowest;
		highest = x > highest ? x : highest;
	}
	return highest - lowest;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		sum += checksum(line);
	}
	printf("%d\n", sum);
	free(line);
	return 0;
}
