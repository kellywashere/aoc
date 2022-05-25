#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
	int          x;
	int          y;
	int          size;
	int          used;
	struct node* next;
};

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && (*line < '0' || *line > '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int main(int argc, char* argv[]) {
	struct node* nodelist = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (line[0] == '/') {
			struct node* n = malloc(sizeof(struct node));
			char* l = line;
			n->x = read_int(&l);
			n->y = read_int(&l);
			n->size = read_int(&l);
			n->used = read_int(&l);
			n->next = nodelist;
			nodelist = n;
		}
	}
	int count = 0;
	for (struct node* na = nodelist; na != NULL; na = na->next) {
		for (struct node* nb = nodelist; nb != NULL; nb = nb->next) {
			if (na != nb && na->used > 0 && (nb->used + na->used <= nb->size))
				++count;
		}
	}
	printf("%d\n", count);

	while (nodelist) {
		struct node* n = nodelist;
		nodelist = nodelist->next;
		free(n);
	}
	free(line);
	return 0;
}
