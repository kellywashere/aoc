#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

const char* openbrack  = "([{<";
const char* closebrack = ")]}>";
const int   scores[] = {3, 57, 1197, 25137};

#define MAX_STACK_SIZE 256
int process_line(const char* line) {
	int stack[MAX_STACK_SIZE];
	int stackptr = 0;
	while (*line) {
		char* b = strchr(openbrack, *line);
		if (b) // open bracket
			stack[stackptr++] = b - openbrack;
		else { // close bracket?
			b = strchr(closebrack, *line);
			if (b) { // yes
				if (stackptr > 0) {
					int stackbrack = stack[--stackptr];
					if (stackbrack != b - closebrack) // mismatch
						return scores[b - closebrack];
				}
				else { // empty stack should not be part of this problem
					fprintf(stderr, "Unexpected closing bracket %c\n", *line);
					return 0;
				}
			}
		}
		++line;
	}
	return 0;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int score = 0;
	while (getline(&line, &len, stdin) != -1) {
		score += process_line(line);
	}
	printf("%d\n", score);
	free(line);
	return 0;
}

