#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

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

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int max_sum = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line)) {
			max_sum = sum > max_sum ? sum : max_sum;
			sum = 0;
		}
		else
			sum += read_int(&l);
	}
	max_sum = sum > max_sum ? sum : max_sum;
	free(line);

	printf("%d\n", max_sum);
	return 0;
}
