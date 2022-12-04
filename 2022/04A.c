#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

struct range {
	int low;
	int high;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
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
	return num;
}

bool contains(struct range* r1, struct range* r2) {
	// true if r1 contains r2
	return (r1->low <= r2->low && r1->high >= r2->high);
}

int main(int argc, char* argv[]) {
	struct range r1, r2;
	int count = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		 r1.low = read_int(&l);
		 r1.high = read_int(&l);
		 r2.low = read_int(&l);
		 r2.high = read_int(&l);
		 count += (contains(&r1, &r2) || contains (&r2, &r1)) ? 1 : 0;
	}
	free(line);

	printf("%d\n", count);
	return 0;
}
