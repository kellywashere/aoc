#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

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

int main(int argc, char* argv[]) {
	int validcount = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int idx1 = read_int(&l);
		int idx2 = read_int(&l);
		// read char in condition
		char c;
		while (*l && *l != ':') {
			c = *l;
			++l;
		}
		// find beginning of pw
		while (*l && (*l < 'a' || *l > 'z'))
			++l;
		// check pw
		int count = l[idx1 - 1] == c ? 1 : 0;
		count += l[idx2 - 1] == c ? 1 : 0;
		validcount += count == 1 ? 1 : 0;
	}
	printf("%d\n", validcount);
	free(line);
	return 0;
}
