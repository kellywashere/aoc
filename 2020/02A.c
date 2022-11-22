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
		int minocc = read_int(&l);
		int maxocc = read_int(&l);
		// read char in condition
		char c;
		while (*l && *l != ':') {
			c = *l;
			++l;
		}
		// count occurrence of c in remainder
		int count = 0;
		while (*l) {
			if (*l == c)
				++count;
			++l;
		}
		if (count >= minocc && count <= maxocc)
			++validcount;
	}
	printf("%d\n", validcount);
	free(line);
	return 0;
}
