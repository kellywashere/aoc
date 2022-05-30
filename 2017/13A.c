#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int severity = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int depth, range;
		read_int(&l, &depth);
		read_int(&l, &range);
		/*
		t = depth;
		period = 2*(range - 1);
		pos(t) = t % period;
		*/
		if (depth % (2*(range - 1)) == 0)
			severity += depth * range;
	}
	printf("%d\n", severity);
	free(line);
	return 0;
}
