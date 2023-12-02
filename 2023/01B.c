#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

char* digstr[] = {"zero", "one", "two", "three", "four", "five",
	"six", "seven", "eight", "nine" };

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_digit_from_line(char** pLine) {
	char* l = *pLine;
	int dig = -1;
	if (isdigit(*l))
		dig = (*l) - '0';
	for (int ii = 0; dig == -1 && ii <= 9; ++ii)
		if (strncmp(l, digstr[ii], strlen(digstr[ii])) == 0)
			dig = ii;
	*pLine = l + 1;
	return dig;
}

int get_int_from_line(char* line) {
	printf("line: %s", line);
	int first = -1;
	int last = -1;

	while (first == -1 && *line)
		first = read_digit_from_line(&line);
	last = first;
	while (*line) {
		int dig = read_digit_from_line(&line);
		last = dig != -1 ? dig : last;
	}
	printf("First: %d, last: %d\n", first, last);
	return 10*first + last;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line))
			sum += get_int_from_line(line);
	}
	free(line);
	printf("%d\n", sum);

	return 0;
}
