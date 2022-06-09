#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRIDSIZE 1000

struct claim {
	int id;
	int x;
	int y;
	int w;
	int h;
	struct claim* next;
};

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int main(int argc, char* argv[]) {
	struct claim* claimlist = NULL;
	int* grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(int));

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		struct claim* c = malloc(sizeof(struct claim));
		c->id = read_int(&l);
		c->x = read_int(&l);
		c->y = read_int(&l);
		c->w = read_int(&l);
		c->h = read_int(&l);
		c->next = claimlist;
		claimlist = c;
		for (int yy = c->y; yy < c->y + c->h; ++yy) {
			int idx = yy * GRIDSIZE + c->x;
			for (int ww = 0; ww < c->w; ++ww)
				++grid[idx++];
		}
	}
	free(line);

	int id = 0;
	for (struct claim* c = claimlist; c != NULL; c = c->next) {
		bool intact = true;
		for (int yy = c->y; yy < c->y + c->h; ++yy) {
			int idx = yy * GRIDSIZE + c->x;
			for (int ww = 0; ww < c->w; ++ww)
				intact = intact && grid[idx++] == 1;
		}
		id = intact ? c->id : id;
	}
	printf("%d\n", id);

	while (claimlist) {
		struct claim* c = claimlist;
		claimlist = claimlist->next;
		free(c);
	}
	free(grid);
	return 0;
}
