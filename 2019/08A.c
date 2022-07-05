#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#define WIDTH 25
#define HEIGHT 6

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char *l = line;
	int digcount[10];
	int minzeros = INT_MAX;
	int answer = 0;
	while (isdigit(*l)) {
		// read new layer
		for (int ii = 0; ii < 10; ++ii)
			digcount[ii] = 0;
		for (int ii = 0; ii < WIDTH * HEIGHT; ++ii) {
			++digcount[*l - '0'];
			++l;
		}
		if (digcount[0] < minzeros) {
			minzeros = digcount[0];
			answer = digcount[1] * digcount[2];
		}
	}
	printf("%d\n", answer);

	free(line);
	return 0;
}
