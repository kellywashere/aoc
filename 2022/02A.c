#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// 0: rock, 1: paper, 2: scissors

// outcome table                             |
const int outcome[] = {  // R  P  S <- you   V opponent
                            0, 1, -1,     // R
                           -1, 0,  1,     // P
                            1,-1,  0 };   // S

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int score = 0;
	while (getline(&line, &len, stdin) != -1) {
		int opponent = line[0] - 'A';
		int you = line[2] - 'X';
		int win = outcome[3 * opponent + you];
		score += (1 + you) + 3 * (win + 1);
	}
	free(line);
	printf("%d\n", score);

	return 0;
}
