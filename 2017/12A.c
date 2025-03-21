#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAXNODES 4096

struct int_list { // for LL of node nrs
	int              val;
	struct int_list* next;
};

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

int groupsize(struct int_list** nodes) {
	// dfs
	int* stack = malloc(MAXNODES * sizeof(int));
	bool* visited = calloc(MAXNODES, sizeof(bool));
	int sp = 0;
	int count = 0;
	// push node 0
	stack[sp++] = 0;
	while (sp) {
		int idx = stack[--sp];
		if (!visited[idx]) {
			visited[idx] = true;
			++count;
			for (struct int_list* n = nodes[idx]; n; n = n->next) {
				stack[sp++] = n->val;
			}
		}
	}
	free(stack);
	free(visited);
	return count;
}

int main(int argc, char* argv[]) {
	struct int_list** nodes = calloc(MAXNODES, sizeof(struct int_list*));
	int nrnodes = 0;
	
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int nodenr;
		read_int(&l, &nodenr);
		if (nodenr != nrnodes)
			fprintf(stderr, "Unexpected node nr %d\n", nodenr);
		nodes[nrnodes] = NULL;
		while (read_int(&l, &nodenr)) {
			struct int_list* n = malloc(sizeof(struct int_list));
			n->val = nodenr;
			n->next = nodes[nrnodes];
			nodes[nrnodes] = n;
		}
		++nrnodes;
	}
	free(line);
	int size = groupsize(nodes);
	printf("%d\n", size);
	// clean up
	for (int ii = 0; ii < nrnodes; ++ii) {
		while (nodes[ii]) {
			struct int_list* n = nodes[ii];
			nodes[ii] = n->next;
			free(n);
		}
	}
	free(nodes);
	return 0;
}
