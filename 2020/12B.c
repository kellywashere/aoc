#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define ABS(x) ((x)>0?(x):-(x))

void rotate(int deg, int* x, int* y) {
	int q = deg / 90;
	if (q * 90 != deg)
		fprintf(stderr, "%d not multiple of 90\n", deg);
	else {
		q = (q + 4) % 4;
		// rotate by 90 deg q times
		for (int r = 0; r < q; ++r) {
			// (x,y) := (y, -x)
			int t = *x;
			*x = *y;
			*y = -t;
		}
	}
}

int main(int argc, char* argv[]) {
	int x = 0;
	int y = 0;

	// waypoint
	int wpx = 10;
	int wpy = -1;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		char instr = *l;
		++l;
		int amount = 0;
		while (isdigit(*l)) {
			amount = 10 * amount + *l - '0';
			++l;
		}
		switch (instr) {
			case 'N':
				wpy -= amount; break;
			case 'E':
				wpx += amount; break;
			case 'S':
				wpy += amount; break;
			case 'W':
				wpx -= amount; break;
			case 'L':
				rotate(amount, &wpx, &wpy); break;
			case 'R':
				rotate(-amount, &wpx, &wpy); break;
			case 'F':
				x += amount * wpx;
				y += amount * wpy;
				break;
			default:
				printf("Invalid instruction `%c`\n", instr);
		}
		// printf("%d, %d\n", x, y);
	}
	printf("%d\n", ABS(x) + ABS(y));
	free(line);
	return 0;
}
