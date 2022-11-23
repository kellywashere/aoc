#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define ABS(x) ((x)>0?(x):-(x))

int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, -1, 0, 1};

int rotate(int dir, int deg) {
	int q = deg / 90;
	if (q * 90 != deg) {
		fprintf(stderr, "%d not multiple of 90\n", dir);
		return dir;
	}
	return ((dir + 4 + q) % 4);
}

int main(int argc, char* argv[]) {
	int x = 0;
	int y = 0;
	int dir = 0;

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
				y -= amount; break;
			case 'E':
				x += amount; break;
			case 'S':
				y += amount; break;
			case 'W':
				x -= amount; break;
			case 'L':
				dir = rotate(dir, amount); break;
			case 'R':
				dir = rotate(dir, -amount); break;
			case 'F':
				x += amount * dir2dx[dir];
				y += amount * dir2dy[dir];
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
