#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int xy_to_keyval(int x, int y) {
	return 3*y + x + 1;
}

int main(int argc, char* argv[]) {
	int code = 0;
	int x = 1;
	int y = 1;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		for (int ii = 0; line[ii] != '\0'; ++ii) {
			if (line[ii] == 'U' && y > 0)
				--y;
			if (line[ii] == 'D' && y < 2)
				++y;
			if (line[ii] == 'L' && x > 0)
				--x;
			if (line[ii] == 'R' && x < 2)
				++x;
		}
		int d = xy_to_keyval(x, y);
		code = code * 10 + d;
	}
	free(line);
	printf("%d\n", code);
	return 0;
}
