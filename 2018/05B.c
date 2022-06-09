#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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

void strcpy_without(char* cpy, char* line, char c) {
	c = tolower(c);
	char C = toupper(c);
	while (*line) {
		while (*line == c || *line == C)
			++line;
		if (*line)
			*cpy++ = *line++;
	}
	*cpy = '\0';
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char* l = line;
	while (*l && *l != '\n')
		++l;
	*l = '\0';

	int linelen = strlen(line);
	char* cpy = malloc((linelen + 1) * sizeof(char));
	int shortest_len = linelen;
	for (char c = 'A'; c <= 'Z'; ++c) {
		strcpy_without(cpy, line, c);
		int newlen = react(cpy);
		shortest_len = newlen < shortest_len ? newlen : shortest_len;
	}

	printf("%d\n", shortest_len);

	free(line);
	free(cpy);
	return 0;
}
