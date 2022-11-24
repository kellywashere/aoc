#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

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
	*pLine = line;
	*x = num;
	return true;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	// read time from first line
	getline(&line, &len, stdin);
	char* l = line;
	int time;
	read_int(&l, &time);

	// read IDs from second line
	getline(&line, &len, stdin);
	l = line;
	int id = 0;
	int twait_min = INT_MAX;
	int id_min = 0;
	while (read_int(&l, &id)) {
		int twait = id - (time % id);
		if (twait < twait_min) {
			twait_min = twait;
			id_min = id;
		}
	}
	printf("%d\n", id_min * twait_min);

	free(line);
	return 0;
}
