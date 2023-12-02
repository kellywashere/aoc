#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

int get_int_from_line(char* line) {
	int first, last;

	for (first = -1; *line && first == -1; ++line)
		first = isdigit(*line) ? (*line) - '0' : first;
	for (last = first; *line; ++line)
		last = isdigit(*line) ? (*line) - '0' : last;
	return 10*first + last;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1)
		sum += get_int_from_line(line);
	free(line);
	printf("%d\n", sum);

	return 0;
}
