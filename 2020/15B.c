#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define TURNS 30000000

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	*x = num;
	return true;
}

/*
int say(int n, int turn, int* turn_spoken) {
	// it is now turn `turn`. Say: n, calculate next number and return that
	// int next = turn_spoken[n] ? turn - turn_spoken[n] : 0;
	int next;
	printf("TURN %d:\n", turn);
	printf("  About to say %d on turn %d\n", n, turn);
	if (turn_spoken[n]) {
		printf("  Nr %d was said before on turn %d\n", n, turn_spoken[n]);
		next = turn - turn_spoken[n];
	}
	else {
		printf("  Nr %d was not said before\n", n);
		next = 0;
	}
	printf("  Therefore: number after my turn will be %d\n", next);
	turn_spoken[n] = turn;
	printf("  -- `%d` -- !\n", n);
	// printf("Turn %d: %d (next: %d)\n", turn, n, next);
	return next;
}
*/

int say(int n, int turn, int* turn_spoken) {
	// it is now turn `turn`. Say: n, calculate next number and return that
	int next = turn_spoken[n] ? turn - turn_spoken[n] : 0;
	turn_spoken[n] = turn;
	return next;
}

int main(int argc, char* argv[]) {
	int* turn_spoken = calloc(TURNS, sizeof(int));

	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	char* l = line;
	int n;
	int turn = 1;
	int next = 0; // next number to be spoken
	while (read_int(&l, &n)) {
		next = say(n, turn, turn_spoken);
		++turn;
	}

	for (; turn <= TURNS; ++turn) {
		n = next;
		next = say(n, turn, turn_spoken);
	}
	printf("%d\n", n);

	free(line);
	free(turn_spoken);
	return 0;
}
