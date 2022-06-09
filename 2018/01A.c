#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		sum += read_int(&l);
	}
	free(line);
	printf("%d\n", sum);

	return 0;
}
