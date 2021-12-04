#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int count_delta(const char* str) {
	int count = 2; // surrounding quotes
	while (*str) {
		if (*str == '"' || *str == '\\')
			++count;
		++str;
	}
	return count;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		count += count_delta(line);
	}
	free(line);
	printf("%d\n", count);

	return 0;
}
