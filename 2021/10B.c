#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

const char* openbrack  = "([{<";
const char* closebrack = ")]}>";

#define MAX_STACK_SIZE 256
uint64_t process_line(const char* line) {
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
						return 0; // discard corrupted lines
				}
				else { // empty stack should not be part of this problem
					fprintf(stderr, "Unexpected closing bracket %c\n", *line);
					return 0;
				}
			}
		}
		++line;
	}
	// autocomplete
	uint64_t score = 0;
	while (stackptr) {
		int stackbrack = stack[--stackptr];
		score = score * 5 + stackbrack + 1;
	}
	return score;
}

int cmp(const void* a, const void* b) {
	return (*(uint64_t*)a > *(uint64_t*)b) ? 1 : -1;
}

#define MAX_SCORES 1024
int main(int argc, char* argv[]) {
	uint64_t scores[MAX_SCORES];
	int nr_scores = 0;
	char *line = NULL;
	size_t len = 0;
	uint64_t score = 0;
	while (getline(&line, &len, stdin) != -1) {
		score = process_line(line);
		if (score)
			scores[nr_scores++] = score;
	}
	qsort(scores, nr_scores, sizeof(uint64_t), cmp);
	printf("%" PRIu64 "\n", scores[nr_scores/2]);
	free(line);
	return 0;
}

