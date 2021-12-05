#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int h = 0;
	int v = 0;
	while (getline(&line, &len, stdin) != -1) {
		char c = line[0];
		char* s = line + 1;
		while (*s < '0' || *s > '9')
			++s;
		int x = atoi(s);
		if (c == 'f')
			h += x;
		else if (c == 'd')
			v += x;
		else if (c== 'u')
			v -= x;
		else
			fprintf(stderr, "Unknown cmd %s", line);
	}
	free(line);
	printf("%d\n", h * v);
	return 0;
}
