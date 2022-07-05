#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#define WIDTH 25
#define HEIGHT 6

int main(int argc, char* argv[]) {
	int pixels[WIDTH * HEIGHT];

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	int nrdigits = 0;
	for ( ; isdigit(line[nrdigits]); ++nrdigits)
		;
	int nrlayers = nrdigits / (WIDTH * HEIGHT);

	for (int ii = 0; ii < WIDTH * HEIGHT; ++ii) {
		pixels[ii] = 2;
		for (int layer = 0; pixels[ii] == 2 && layer < nrlayers; ++layer)
			pixels[ii] = line[ii + layer * WIDTH * HEIGHT] - '0';
	}
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x)
			printf("%c", pixels[y * WIDTH + x] == 1 ? '#' : ' ');
		printf("\n");
	}

	free(line);
	return 0;
}
