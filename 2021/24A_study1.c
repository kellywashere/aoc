#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

int main(int argc, char* argv[]) {
	char lines[300][40];
	char *line = NULL;
	size_t len = 0;
	int linenr = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		strncpy(lines[linenr], line, 40);
		++linenr;
	}
	for (int offset = 0; offset < 18; ++offset)
		for (int ii = 0; ii < linenr; ii += 18)
			printf("%s", lines[ii + offset]);
	free(line);
	return 0;
}
