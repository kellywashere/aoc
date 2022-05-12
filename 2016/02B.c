#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int keylut[] = {0,  0,  1,  0,  0,
                0,  2,  3,  4,  0,
                5,  6,  7,  8,  9,
                0, 10, 11, 12,  0,
                0,  0, 13,  0,  0};

int main(int argc, char* argv[]) {
	int code = 0;
	int keyidx = 10; // start at nr 5

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		for (int ii = 0; line[ii] != '\0'; ++ii) {
			if (line[ii] == 'U' && keyidx >= 5 && keylut[keyidx - 5])
				keyidx -= 5;
			if (line[ii] == 'D' && keyidx < 20 && keylut[keyidx + 5])
				keyidx += 5;
			if (line[ii] == 'L' && keyidx > 0 && keylut[keyidx - 1])
				--keyidx;
			if (line[ii] == 'R' && keyidx < 24 && keylut[keyidx + 1])
				++keyidx;
		}
		code = (code << 4) + keylut[keyidx];
	}
	free(line);
	printf("%X\n", code);
	return 0;
}
