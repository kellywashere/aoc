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

int calc_metadata_sum(char** pLine) {
	int sum = 0;
	int nr_children = read_int(pLine);
	int nr_meta = read_int(pLine);
	for (int ii = 0; ii < nr_children; ++ii)
		sum += calc_metadata_sum(pLine);
	for (int ii = 0; ii < nr_meta; ++ii)
		sum += read_int(pLine); // read metadata of this node
	return sum;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char* l = line;
	int sum = calc_metadata_sum(&l);
	printf("%d\n", sum);

	free(line);
	return 0;
}
