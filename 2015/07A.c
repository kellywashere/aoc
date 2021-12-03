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
	int          in1; // <0: -idx-1 of wire, >=0: nr
	int          in2; // not used in some ops
	int          out; // -idx-1 output wire (neg for consistency...)
	struct gate* next; // for linked list
};

struct wire {
	int known_bits; // mask of resolved bits
	int value;
};

// debug:
void print_wire(struct wire* w) {
	printf("v:%04X, k:%04X --> ", w->value, w->known_bits);
	for (int b = 15; b >= 0; --b) {
		bool k = ((w->known_bits >> b) & 1);
		int v = ((w->value >> b) & 1);
		fputc(k ? (v + '0') : '?', stdout);
	}
}

//debug:
void print_wirename(int idx) {
	if (idx < 26)
		fputc(idx + 'a', stdout);
	else {
		fputc((idx / 26) + 'a' - 1, stdout);
		fputc((idx % 26) + 'a', stdout);
	}
}

//debug:
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

bool all_whitespace(const char* str) {
	while (*str == ' ' || *str == '\t' || *str == '\n')
		++str;
	return *str == '\0';
}

int wirename_to_idx(const char* name) {
	// returns 0..701
	char c1 = *name;
	char c2 = *(name + 1);
	if (c2 >= 'a' && c2 <='z')
		return (c1 - 'a' + 1)*26 + (c2 - 'a'); // 26 .. 701
	else
		return c1 - 'a'; // 0 .. 25
}

int parse_operand(const char* str) {
	// when nr, returns that nr
	// when wire name, returns -idx - 1
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
	gate->in1 = 0;
	gate->in2 = 0;

	char* arrowloc = strstr(line, "->");
	if (arrowloc == NULL) {
		fprintf(stderr, "Did not find -> in line %s", line);
		return NULL;
	}
	gate->out = parse_operand(arrowloc + 2);
	if (gate->out >= 0) {
		fprintf(stderr, "Gate output should always be wire name: %s", line);
		return NULL;
	}

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

bool process_gate(struct gate* gate, struct wire* wires) {
	struct wire* w_out = wires + (-gate->out - 1);
	struct wire* w_in1 = gate->in1 < 0 ? wires + (-gate->in1 - 1) : NULL;
	struct wire* w_in2 = gate->in2 < 0 ? wires + (-gate->in2 - 1) : NULL;
	int v1 = gate->in1 < 0 ? w_in1->value : gate->in1;
	int v2 = gate->in2 < 0 ? w_in2->value : gate->in2;
	int k1 = gate->in1 < 0 ? w_in1->known_bits : 0xFFFF;
	int k2 = gate->in2 < 0 ? w_in2->known_bits : 0xFFFF;
	int kout_before = w_out->known_bits;

	switch (gate->op) {
		case NOP:
			w_out->known_bits = k1;
			w_out->value = v1;
			break;
		case NOT:
			w_out->known_bits = k1;
			w_out->value = ~v1;
			break;
		case LSHIFT:
			w_out->known_bits = k1 << v2;
			w_out->known_bits |= (1 << v2) - 1; // we know last v2 bits are 0
			w_out->value = v1 << v2;
			break;
		case RSHIFT:
			w_out->known_bits = k1 >> v2;
			w_out->known_bits |= 0xFFFF << (16 - v2); // we know first v2 bits are 0
			w_out->value = v1 >> v2;
			break;
		case AND:
			// ? * ? = ?              v1 k1  v2 k2  vo ko 
			// 0 * ? = ? * 0 = 0       -  0   -  0   -  0
			// 1 * ? = ? * 1 = ?       0  1   -  -   0  1
			// 1 * 1 = 1               -  -   0  1   0  1
			//                         1  1   -  0   -  0
			//                         -  0   1  1   -  0
			//                         1  1   1  1   1  1
			// So: vo = v1*k1*v2*k2
			// and ko = ~v1*k1 + ~v2*k2 + v1*k1*v2*k2
			w_out->value = v1 & k1 & v2 & k2;
			w_out->known_bits = (~v1 & k1) | (~v2 & k2) | w_out->value;
			break;
		case OR:
			// ? + ? = ?              v1 k1  v2 k2  vo ko 
			// 0 + ? = ? + 0 = ?       -  0   -  0   -  0
			// 1 + ? = ? + 1 = 1       1  1   -  -   1  1
			// 0 + 0 = 0               -  -   1  1   1  1
			//                         0  1   -  0   -  0
			//                         -  0   0  1   -  0
			//                         0  1   0  1   0  1
			// So: vo = v1*k1 + v2*k2
			// and ko = v1*k1 + v2*k2 + ~v1*k1*~v2*k2
			w_out->value = (v1 & k1) |  (v2 & k2);
			w_out->known_bits = (~v1 & k1 & ~v2 & k2) | w_out->value;
			break;
	}
	w_out->value &= 0xFFFF;
	w_out->known_bits &= 0xFFFF;
	return kout_before != w_out->known_bits; // return true if we learned something
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
		if (all_whitespace(line))
			continue;
		struct gate* gate = parse_line(line);
		gate->next = head; // stick new gate in front of list
		head = gate;
	}
	free(line);

	// now repeatedly iterate over all gates, until wite `a` is known
	bool learned = true;
	while (learned) {
		learned = false;
		for (struct gate* gate = head; gate != NULL; gate = gate->next) {
			learned = process_gate(gate, wires) || learned;
		}
	}

	// debug:
	for (int idx = 0; idx <= 701; ++idx) {
		if (wires[idx].known_bits) {
			print_wirename(idx);
			printf(" = %d", wires[idx].value);
			if (wires[idx].known_bits != 0xFFFF)
				fputs(" ??????", stdout);
			fputc('\n', stdout);
		}
	}

	// print answer
	int idx_a = wirename_to_idx("a");
	printf("%d\n", wires[idx_a].value);

	// clean up
	while (head) {
		struct gate* gate = head;
		head = gate->next;
		free(gate);
	}
	free(wires);
	return 0;
}
