#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define OFFSET ('a'-'A')

int react(char* line) {
	int linelen = strlen(line);
	bool changed = true;
	while (changed) {
		changed = false;
		int ii = 0;
		while (ii < linelen - 1) {
			if (line[ii] == line[ii + 1] + OFFSET || line[ii + 1] == line[ii] + OFFSET) {
				changed = true;
				if (linelen - ii > 1)
					memmove(line + ii, line + ii + 2, linelen - ii - 1);
				linelen -= 2;
				if (ii > 0)
					--ii;
			}
			else
				++ii;
		}
	}
	return linelen;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char* l = line;
	while (*l && *l != '\n')
		++l;
	*l = '\0';

	int linelen = react(line);
	printf("%d\n", linelen);

	free(line);
	return 0;
}
