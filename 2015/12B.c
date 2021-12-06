#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void remove_red(char* line) {
	char* s;
	while ( (s = strstr(line, "\":\"red\"")) != NULL) {
		// find corresponding {
		int depth = 0;
		char* s1 = s;
		while (!(*s1 == '{' && depth == 0) && s1 >= line) {
			if (*s1 == '}')
				++depth;
			else if (*s1 == '{')
				--depth;
			--s1;
		}
		// find closing }
		depth = 0;
		char* s2 = s;
		while (!(*s2 == '}' && depth == 0) && *s2) {
			if (*s2 == '{')
				++depth;
			else if (*s2 == '}')
				--depth;
			++s2;
		}
		// replace by spaces
		while (s1 <= s2)
			*s1++ = ' ';
	}
}

int main(int argc, char* argv[]) {
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* s = line;
		// hacky step 1: replace all objects with red value by spaces
		remove_red(line);
		while (*s) {
			while (*s && *s != '-' && (*s < '0' || *s > '9'))
				++s;
			if (*s) {
				sum += strtol(s, &s, 10);
			}
		}
	}
	free(line);
	printf("%d\n", sum);
	return 0;
}
