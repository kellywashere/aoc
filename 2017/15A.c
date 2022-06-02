#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>

bool read_int(char** pLine, uint64_t* x) {
	uint64_t num = 0;
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
	uint64_t p = 2147483647;
	uint64_t fa = 16807;
	uint64_t fb = 48271;

	uint64_t a = 0;
	uint64_t b = 0;

	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	char* l = line;
	read_int(&l, &a);

	getline(&line, &len, stdin);
	l = line;
	read_int(&l, &b);

	free(line);

	int count = 0;
	for (int ii = 0; ii < 40000000; ++ii) {
		a = (a * fa) % p;
		b = (b * fb) % p;
		// printf("%24" PRIu64 "%24" PRIu64 "\n", a, b);
		if ((a & 0xffff) == (b & 0xffff))
			++count;
	}
	printf("%d\n", count);

	return 0;
}
