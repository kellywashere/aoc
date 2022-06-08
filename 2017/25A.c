#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_STATES 16

struct instr {
	bool writeval;
	int  movedir;
	int  nextstate;
};

struct state {
	struct instr instr[2];
};

struct tape {
	bool* tape;
	int   len;
	int   pos;
	int   offs; // idx = offs + pos
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

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
	*x = num;
	*pLine = line;
	return true;
}

struct tape* create_tape(int len) {
	struct tape* t = malloc(sizeof(struct tape));
	t->tape = calloc(len, sizeof(bool));
	t->len = len;
	t->pos = 0;
	t->offs = len/2;
	return t;
}

void destroy_tape(struct tape* t) {
	if (t) {
		free(t->tape);
		free(t);
	}
}

void tape_write(struct tape* t, bool v) {
	t->tape[t->pos + t->offs] = v;
}

bool tape_read(struct tape* t) {
	return t->tape[t->pos + t->offs];
}

void tape_move(struct tape* t, int dir) {
	t->pos += dir;
	int idx = t->pos + t->offs;
	if (idx < 0) { // add tape to left
		t->tape = realloc(t->tape, 2*t->len);
		for (int ii = 0; ii < t->len; ++ii) {
			t->tape[ii + t->len] = t->tape[ii];
			t->tape[ii] = false;
		}
		t->offs += t->len;
		t->len *= 2;
	}
	else if (idx >= t->len) {
		t->tape = realloc(t->tape, 2*t->len);
		for (int ii = t->len; ii < 2*t->len; ++ii)
			t->tape[ii] = false;
		t->len *= 2;
	}
}

int tape_checksum(struct tape* t) {
	int count = 0;
	for (int ii = 0; ii < t->len; ++ii)
		count += t->tape[ii] ? 1 : 0;
	return count;
}

void show_tape(struct tape* t) {
	for (int ii = 0; ii < t->len; ++ii)
		printf("%c", t->tape[ii] ? '1' : '0');
	printf("\n");
	for (int ii = 0; ii < t->pos + t->offs; ++ii)
		printf(" ");
	printf("^\n");
}

void read_machine_from_stdin(struct state* states, int* nr_states, int* begin_state, int* checksum_after) {
	char *line = NULL;
	size_t len = 0;
	*nr_states = 0;
	// FSM
	int in_state = -1;
	int in_value = -1;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line))
			continue;
		if (!strncmp(line, "Begin", 5))
			*begin_state = *(strstr(line, "state") + 6) - 'A';
		else if (!strncmp(line, "Perform", 7))
			read_int(&l, checksum_after);
		else if (!strncmp(line, "In state", 8)) {
			in_state = line[9] - 'A';
			if (*nr_states < in_state + 1)
				*nr_states = in_state + 1;
		}
		else if (strstr(line, "current value"))
			read_int(&l, &in_value);
		else if (strstr(line, "Write")) {
			int x;
			read_int(&l, &x);
			states[in_state].instr[in_value].writeval = x != 0;
		}
		else if (strstr(line, "Move"))
			states[in_state].instr[in_value].movedir = strstr(line, "left") ? -1 : 1;
		else if (strstr(line, "Continue"))
			states[in_state].instr[in_value].nextstate = *(strstr(line, "state") + 6) - 'A';
	}
	free(line);
}


int main(int argc, char* argv[]) {
	struct state states[MAX_STATES];
	int nr_states = 0;
	int begin_state = -1;
	int checksum_after = -1;

	read_machine_from_stdin(states, &nr_states, &begin_state, &checksum_after);

	struct tape* tape = create_tape(16);

	int state = begin_state;
	for (int ii = 0; ii < checksum_after; ++ii) {
		int v = tape_read(tape) ? 1 : 0;
		tape_write(tape, states[state].instr[v].writeval);
		tape_move(tape, states[state].instr[v].movedir);
		state = states[state].instr[v].nextstate;
	}
	printf("%d\n", tape_checksum(tape));

	destroy_tape(tape);


	return 0;
}
