#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// using the fact that input file wire names are max 2 letters
// for indexing (no hash table needed; no collisions)

#define NR_WIRES (27*26)

enum op {
	NOP, /* no operation */
	NOT,
	LSHIFT,
	RSHIFT,
	AND,
	OR
};

struct gate {
	enum op      op;
	int          in1; // <0: -idx of wire, >=0: nr
	int          in2; // not used in some ops
	int          out; // -idx output wire (neg for consistency...)
	struct gate* next; // for linked list
};

//debug:
void print_wirename(int idx) {
	if (idx < 26)
		fputc(idx + 'a', stdout);
	else {
		fputc((idx / 26) + 'a' - 1, stdout);
		fputc((idx % 26) + 'a', stdout);
	}
}

void print_operand(int o) {
	if (o >= 0)
		printf("%d", o);
	else
		print_wirename(-o - 1);
}

//debug:
void print_gate(struct gate* g) {
	if (g->op == NOT)
		fputc('~', stdout);
	print_operand(g->in1);
	if (g->op == LSHIFT)
		fputs(" << ", stdout);
	else if (g->op == RSHIFT)
		fputs(" >> ", stdout);
	else if (g->op == AND)
		fputs(" * ", stdout);
	else if (g->op == OR)
		fputs(" + ", stdout);
	if (g->op > NOT)
		print_operand(g->in2);
	fputs(" -> ", stdout);
	print_operand(g->out);
	fputc('\n', stdout);
}

struct wire {
	int known_bits; // mask of resolved bits
	int value;
};

int wirename_to_idx(const char* name) {
	char c1 = *name;
	char c2 = *(name + 1);
	if (c2 >= 'a' && c2 <='z')
		return (c1 - 'a' + 1)*26 + (c2 - 'a'); // 26 .. 701
	else
		return c1 - 'a'; // 0 .. 25
}

int parse_operand(const char* str) {
	while (*str == ' ' || *str == '\t' || *str == '\n')
		++str;
	if (*str >= '0' && *str <= '9')
		return (int)strtol(str, NULL, 10);
	else
		return -wirename_to_idx(str) - 1;
}

struct gate* parse_line(const char* line) {
	struct gate* gate = malloc(sizeof(struct gate));
	gate->next = NULL;

	char* arrowloc = strstr(line, "->");
	if (arrowloc == NULL) {
		fprintf(stderr, "Did not find -> in line %s", line);
		return NULL;
	}
	gate->out = parse_operand(arrowloc + 2);

	char* oploc;
	if (!strncmp(line, "NOT", 3)) {
		gate->op = NOT;
		gate->in1 = parse_operand(line + 4);
	}
	else if ( (oploc = strstr(line, "LSHIFT")) != NULL) {
		gate->op = LSHIFT;
		gate->in1 = parse_operand(line);
		gate->in2 = parse_operand(oploc + 7);
	}
	else if ( (oploc = strstr(line, "RSHIFT")) != NULL) {
		gate->op = RSHIFT;
		gate->in1 = parse_operand(line);
		gate->in2 = parse_operand(oploc + 7);
	}
	else if ( (oploc = strstr(line, "AND")) != NULL) {
		gate->op = AND;
		gate->in1 = parse_operand(line);
		gate->in2 = parse_operand(oploc + 4);
	}
	else if ( (oploc = strstr(line, "OR")) != NULL) {
		gate->op = OR;
		gate->in1 = parse_operand(line);
		gate->in2 = parse_operand(oploc + 3);
	}
	else { // NOP
		gate->op = NOP;
		gate->in1 = parse_operand(line);
	}
	return gate;
}

int main(int argc, char* argv[]) {
	int ii;
	// create wires with unknown value
	struct wire* wires = malloc(NR_WIRES * sizeof(struct wire));
	for (ii = 0; ii < NR_WIRES; ++ii) {
		wires[ii].known_bits = 0; // no bits known
		wires[ii].value = 0;
	}

	char *line = NULL;
	size_t len = 0;
	struct gate* head = NULL; // head of linked list
	while (getline(&line, &len, stdin) != -1) {
		struct gate* gate = parse_line(line);
		gate->next = head; // stick new gate in front of list
		head = gate;
		fputs(line, stdout);
		print_gate(gate);
	}
	free(line);
	
	// clean up
	while (head) {
		struct gate* gate = head;
		head = gate->next;
		free(gate);
	}
	return 0;
}
