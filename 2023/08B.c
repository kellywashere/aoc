#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct node {
	int left;
	int right;
};

uint64_t gcd(uint64_t a, uint64_t b) {
	while (a != 0 && b != 0) {
		uint64_t na = b;
		b = a % b;
		a = na;
	}
	return b == 0 ? a : b;
}

uint64_t lcm(uint64_t a, uint64_t b) {
	return a * b / gcd(a, b);
}


#define MAX_INSTR_LEN 1024
#define MAX_NODES (26*26*26)
#define ZZZ (26*26*26 - 1)

int read_node(char** pLine) {
	char* l = *pLine;
	int node = 0;
	while (*l < 'A' || *l > 'Z')
		++l;
	for (int ii = 0; ii < 3; ++ii) {
		node = 26 * node + *l - 'A';
		++l;
	}
	*pLine = l;
	return node;
}

void node2str(int node, char* str) {
	for (int ii = 2; ii >= 0; --ii) {
		str[ii] = (node % 26) + 'A';
		node /= 26;
	}
}

struct state {
	int node;
	int instridx;
};

/*
int f(int x) {
	++x;
	if (x > 100)
		x -= 13;
	return x;
}

void floyd(int x0, int* mu, int* lambda) {
	// https://en.wikipedia.org/wiki/Cycle_detection
	int tortoise = f(x0);
	int hare = f(tortoise);

	while (tortoise != hare) {
		tortoise = f(tortoise);
		hare = f(f(hare));
	}

	*mu = 0;
	tortoise = x0;
	while (tortoise != hare) {
		tortoise = f(tortoise);
		hare = f(hare);
		++(*mu);
	}

	*lambda = 1;
	hare = f(tortoise);
	while (tortoise != hare) {
		hare = f(hare);
		++(*lambda);
	}
}
*/

void make_step(struct state* s, struct node nodes[], char* instr) {
	s->node = instr[s->instridx] == 'L' ? nodes[s->node].left : nodes[s->node].right;
	++s->instridx;
	if (!instr[s->instridx])
		s->instridx = 0;
}

void floyd(struct state* x0, struct node nodes[], char* instr, int* mu, int* lambda) {
	// https://en.wikipedia.org/wiki/Cycle_detection
	struct state tortoise = *x0;
	make_step(&tortoise, nodes, instr); // tortoise = f(x0);
	struct state hare = tortoise;
	make_step(&hare, nodes, instr); // hare = f(tortoise);

	while (tortoise.node != hare.node || tortoise.instridx != hare.instridx) {
		make_step(&tortoise, nodes, instr); // tortoise = f(tortoise);
		make_step(&hare, nodes, instr);
		make_step(&hare, nodes, instr); // hare = f(f(hare));
	}

	*mu = 0;
	tortoise = *x0;
	while (tortoise.node != hare.node || tortoise.instridx != hare.instridx) {
		make_step(&tortoise, nodes, instr); // tortoise = f(tortoise);
		make_step(&hare, nodes, instr);
		++(*mu);
	}

	*lambda = 1;
	hare = tortoise;
	make_step(&hare, nodes, instr); // hare = f(tortoise);
	while (tortoise.node != hare.node || tortoise.instridx != hare.instridx) {
		make_step(&hare, nodes, instr);
		++(*lambda);
	}
}

int main(int argc, char* argv[]) {
	char instr[MAX_INSTR_LEN] = {0};
	int instr_len = 0;
	struct node nodes[MAX_NODES];
	int Anodes[16];
	int nr_anodes = 0;

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	char *l = line;
	while (*l == 'L' || *l == 'R') {
		instr[instr_len++] = *l;
		++l;
	}
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			l = line;
			int node = read_node(&l);
			nodes[node].left = read_node(&l);
			nodes[node].right = read_node(&l);
			if (node % 26 == 0)
				Anodes[nr_anodes++] = node;
		}
	}
	free(line);

	int mu[16];
	int lambda[16];
	for (int ii = 0; ii < nr_anodes; ++ii) {
		struct state x0;
		x0.node = Anodes[ii];
		x0.instridx = 0;
		floyd(&x0, nodes, instr, &mu[ii], &lambda[ii]);
		/* This proves the comment below:
		printf("mu = %d, lambda = %d\n", mu[ii], lambda[ii]);

		// find first z-node
		int steps_till_znode = 0;
		x0.node = Anodes[ii];
		x0.instridx = 0;
		while (x0.node % 26 != 25) {
			make_step(&x0, nodes, instr);
			++steps_till_znode;
		}
		printf("steps till z: %d\n", steps_till_znode);
		steps_till_znode = 0;
		while (steps_till_znode == 0 || x0.node % 26 != 25) {
			make_step(&x0, nodes, instr);
			++steps_till_znode;
		}
		printf("steps till z: %d\n", steps_till_znode);
		*/
	}
	// path is laid out such that after lambda steps, we reach a Z-node !!!!!
	// this does not have to be true, but it is so we use it!
	// this is not a general solution, therefore...
	// answer is lcm of all lambdas
	uint64_t steps = lcm(lambda[0], lambda[1]);
	for (int ii = 2; ii < nr_anodes; ++ii)
		steps = lcm(steps, lambda[ii]);
	printf("%"PRIu64"\n", steps);

	return 0;
}
