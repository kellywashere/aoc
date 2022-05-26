#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	int linelen = 0;
	while (!isspace(line[linelen]))
		++linelen;
	int sum = 0;
	for (int ii = 0; ii < linelen / 2; ++ii) {
		if (line[ii] == line[ii + linelen/2])
			sum += 2 * (line[ii] - '0');
	}
	printf("%d\n", sum);

	free(line);
	return 0;
}
