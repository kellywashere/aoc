#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool has_specific_seq(const char* str, char c1, char c2) {
	for (; *str; ++str) {
		if (*str == c1 && *(str + 1) == c2)
			return true;
	}
	return false;
}

bool has_double_pair(const char* str) {
	char cprev = *(str++);
	for (; *str; ++str) {
		if (*(str + 1) && has_specific_seq(str + 1, cprev, *str))
			return true;
		cprev = *str;
	}
	return false;
}

bool has_gap_pair(const char* str) {
	for (; *str; ++str) {
		if (*(str + 1) != '\0' && *(str + 2) == *str)
			return true;
	}
	return false;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (has_double_pair(line) && has_gap_pair(line))
			++count;
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
