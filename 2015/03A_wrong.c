#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

enum dir {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3
};

const char* dirstr = "^v<>";

struct node {
	struct node* neighbors[4];
};

enum dir opposite(enum dir d) {
	switch (d) {
		case UP:
			return DOWN;
		case DOWN:
			return UP;
		case LEFT:
			return RIGHT;
		case RIGHT:
			return LEFT;
		default:
			return -1; // unreachable
	}
}

struct node* new_node() {
	int ii;
	struct node* n = malloc(sizeof(struct node));
	for (ii = 0; ii < 4; ++ii)
		n->neighbors[ii] = NULL;
	return n;
}

int main(int argc, char* argv[]) {
	char c;
	int nodes = 0;
	struct node* n = new_node(&nodes);
	++nodes;
	while ( (c = fgetc(stdin)) != EOF) {
		char* cloc = strchr(dirstr, c);
		if (cloc == NULL)
			continue;
		enum dir d = (enum dir)(cloc - dirstr);
		if (n->neighbors[d] == NULL) {
			struct node* nn = new_node();
			++nodes;
			n->neighbors[d] = nn;
			nn->neighbors[opposite(d)] = n;
		}
		n = n->neighbors[d];
	}
	printf("%d\n", nodes);
	return 0;
}
