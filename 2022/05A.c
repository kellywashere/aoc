#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_STACKS 9
#define MAX_CRATES 100

struct stack {
	char* crates;
	int   stack_sz;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
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
	return num;
}

void print_stack(struct stack* stack) {
	for (int ii = 0; ii < stack->stack_sz; ++ii)
		printf("%c", stack->crates[ii]);
	printf("\n");
}

void add_crate_to_bottom(struct stack* stack, char crate) {
	// used only during read in (slow)
	for (int ii = stack->stack_sz - 1; ii >= 0; --ii)
		stack->crates[ii + 1] = stack->crates[ii];
	stack->crates[0] = crate;
	++stack->stack_sz;
}

void move_crates(struct stack* from, struct stack* to, int amount) {
	if (from->stack_sz < amount)
		fprintf(stderr, "Error: not enough crates (%d < %d)\n", from->stack_sz, amount);
	else if (to->stack_sz + amount > MAX_CRATES)
		fprintf(stderr, "Error: not enough room for crates (%d > %d)\n", to->stack_sz + amount, MAX_CRATES);
	else {
		for (int n = 0; n < amount; ++n) {
			char c = from->crates[--from->stack_sz];
			to->crates[to->stack_sz++] = c;
		}
	}
}

int main(int argc, char* argv[]) {
	struct stack stacks[MAX_STACKS];
	for (int ii = 0; ii < MAX_STACKS; ++ii) {
		stacks[ii].crates = malloc(MAX_CRATES * sizeof(char));
		stacks[ii].stack_sz = 0;
	}

	char *line = NULL;
	size_t len = 0;
	// read initial stacks
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		if (line[1] != '1') { // ignore last line of stack description
			size_t llen = strlen(line);
			for (int s = 0; s < MAX_STACKS && (4*s + 1) < llen; ++s)
				if (isupper(line[4*s + 1]))
					add_crate_to_bottom(&stacks[s], line[4*s + 1]);
		}
	}
	// read instructions
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int amount = read_int(&l);
		int fromidx = read_int(&l) - 1;
		int toidx = read_int(&l) - 1;
		move_crates(&stacks[fromidx], &stacks[toidx], amount);
	}
	free(line);

	/*
	for (int s = 0; s < MAX_STACKS; ++s)
		print_stack(stacks + s);
	*/

	for (int s = 0; s < MAX_STACKS; ++s)
		printf("%c", stacks[s].crates[stacks[s].stack_sz - 1]);
	printf("\n");

	for (int ii = 0; ii < MAX_STACKS; ++ii)
		free(stacks[ii].crates);

	return 0;
}
