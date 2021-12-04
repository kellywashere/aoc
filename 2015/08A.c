#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int count_delta(const char* str) {
	int count = 0;
	while (*str != '"')
		++str;
	++str; // skip "
	++count; // and count it
	while (*str != '"') {
		if (*str == '\\') {
			++str;
			switch (*str) {
				case '"':
				case '\\':
					++count;
					break;
				case 'x':
					count += 3;
					str += 2;
					break;
				default:
					fprintf(stderr, "Unexpected char %c\n", *str);
			}
		}
		++str;
	}
	++count; // last "
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
