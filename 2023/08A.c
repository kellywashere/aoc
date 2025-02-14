#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct node {
	int left;
	int right;
};

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
	for (int ii = 0; ii < 3; ++ii) {
		str[ii] = (node % 26) + 'A';
		node /= 26;
	}
}

int main(int argc, char* argv[]) {
	char instr[MAX_INSTR_LEN] = {0};
	int instr_len = 0;
	struct node nodes[MAX_NODES];

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
		}
	}
	free(line);

	int node = 0;
	int instr_idx = 0;
	int steps = 0;
	
	while (node != ZZZ) {
		/*
		char nodestr[3];
		node2str(node, nodestr);
		printf("Currently in node %d %.3s\n", node, nodestr);
		*/
		node = instr[instr_idx] == 'L' ? nodes[node].left : nodes[node].right;
		instr_idx = (instr_idx + 1) % instr_len;
		++steps;
	}
	printf("%d\n", steps);

	return 0;
}
