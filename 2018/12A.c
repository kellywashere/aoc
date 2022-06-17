#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void print_state(bool* state, int len) {
	for (int ii = 0; ii < len; ++ii)
		printf("%c", state[ii] ? '#' : '.');
	printf("\n");
}

int str2ruleidx(char* str) {
	int rule = 0;
	for (int ii = 0; ii < 5; ++ii)
		rule = (rule << 1) + (str[ii] == '#' ? 1 : 0);
	return rule;
}

int boolarray2ruleidx(bool* ba) {
	int rule = 0;
	for (int ii = 0; ii < 5; ++ii)
		rule = (rule << 1) + (ba[ii] ? 1 : 0);
	return rule;
}

int main(int argc, char* argv[]) {
	int steps = 20;

	char *line = NULL;
	size_t len = 0;

	// read initial state
	// 1. find initial length
	getline(&line, &len, stdin);
	char* l = line;
	while (*l && *l != '.' && *l != '#')
		++l;
	char* l2 = l;
	while (*l2 && (*l2 != '.' || *l2 != '#'))
		++l2;
	int lenzero  = (int)(l2 - l);
	// 2. reserve mem
	int lenfinal = lenzero + steps * 4; // worst case length: TODO: check real max extension per step
	bool* state = calloc(lenfinal, sizeof(bool));
	bool* newstate = calloc(lenfinal, sizeof(bool));
	// 3. copy initial state in center of state vector
	for (int ii = 0; ii < lenzero; ++ii)
		state[steps * 2 + ii] = l[ii] == '#';

	// read rules
	bool rules[32] = {false};
	getline(&line, &len, stdin); // empty line
	while (getline(&line, &len, stdin) != -1)
		rules[str2ruleidx(line)] = line[9] == '#';
	free(line);

	// simulate
	for (int step = 1; step <= steps; ++step) {
		memset(newstate, false, lenfinal);
		for (int ii = 2; ii <= lenfinal - 3; ++ii)
			newstate[ii] = rules[boolarray2ruleidx(state + ii - 2)];
		bool* tmp = state;
		state = newstate;
		newstate = tmp;
		// print_state(state, lenfinal);
	}

	// sum idx of 'plants'
	int sum = 0;
	for (int ii = 0; ii < lenfinal; ++ii)
		sum += state[ii] ? ii - 2 * steps : 0;
	printf("%d\n", sum);

	return 0;
}
