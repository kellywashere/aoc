#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* s = line;
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
