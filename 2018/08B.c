#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

#define MAX_CHILDREN 32

int calc_value(char** pLine) {
	int children_values[MAX_CHILDREN];
	int sum = 0;
	int nr_children = read_int(pLine);
	int nr_meta = read_int(pLine);
	for (int ii = 0; ii < nr_children; ++ii)
		children_values[ii] = calc_value(pLine);
	for (int ii = 0; ii < nr_meta; ++ii) {
		int meta = read_int(pLine);
		if (nr_children == 0)
			sum += meta;
		else if (meta > 0 && meta <= nr_children)
			sum += children_values[meta - 1];
	}
	return sum;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char* l = line;
	int value = calc_value(&l);
	printf("%d\n", value);

	free(line);
	return 0;
}
