#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

struct rule {
	int insize; // 2 or 3
	int inval;  // binary encoded input block
	int outval; // binary encoded output block
};

struct grid {
	int  size;
	bool* grid;
};

int rotright2(int x) {
	int r = (x & 4) >> 2;
	r |= (x & 1) << 1;
	r |= (x & 8) >> 1;
	r |= (x & 2) << 2;
	return r;
}

int flipupdn2(int x) {
	return ((x >> 2) &  3) | ((x & 3) << 2);
}

int rotright3(int x) {
	int r = ((x & 64) >> 6) | ((x & 8) >> 2) | ((x & 1) << 2);
	r |= ((x & 128) >> 4) | ((x & 16)) | ((x & 2) << 4);
	r |= ((x & 256) >> 2) | ((x & 32) << 2) | ((x & 4) << 6);
	return r;
}

int flipupdn3(int x) {
	return ((x >> 6) &  7) | (x & 0x38) | ((x & 7) << 6);
}

void print2(int x) {
	printf("%c%c\n", (x>>3)&1 ? '#':'.', (x>>2)&1 ? '#':'.');
	printf("%c%c\n", (x>>1)&1 ? '#':'.', (x>>0)&1 ? '#':'.');
}

void print3(int x) {
	for (int b = 8; b >=0; --b) {
		printf("%c", (x>>b)&1 ? '#':'.');
		if (b%3 == 0)
			printf("\n");
	}
}

void read_block(char** pLine, int* val, int* size) {
	*val = 0;
	if (size)
		*size = 1;
	char* l = *pLine;

	while (*l != '.' && *l != '#')
		++l;
	while (*l == '.' || *l == '#' || *l == '/') {
		if (*l == '/') {
			if (size)
				++(*size);
		}
		else
			*val = (*val << 1) | (*l == '#' ? 1 : 0);
		++l;
	}
	*pLine = l;
}

void read_rule(char* line, struct rule* rule) {
	read_block(&line, &rule->inval, &rule->insize);
	read_block(&line, &rule->outval, NULL);
}

void permutate_rule2(struct rule* rule, int* rules2to3) {
	int x = rule->inval;
	int y = rule->outval;
	for (int ii = 0; ii < 4; ++ii) {
		rules2to3[x] = y;
		rules2to3[flipupdn2(x)] = y;
		x = rotright2(x);
	}
}

void permutate_rule3(struct rule* rule, int* rules3to4) {
	int x = rule->inval;
	int y = rule->outval;
	for (int ii = 0; ii < 4; ++ii) {
		rules3to4[x] = y;
		rules3to4[flipupdn3(x)] = y;
		x = rotright3(x);
	}
}

void apply_rule(struct grid* g, int* rules, int from, int to) {
	// from, to: 2,3 or 3,4
	int nsize = g->size/from * to;
	bool* og = g->grid;
	bool* ng = malloc(nsize * nsize * sizeof(bool));
	for (int y = 0; y < g->size; y += from) {
		for (int x = 0; x < g->size; x += from) {
			// get block value
			int val = 0;
			for (int dy = 0; dy < from; ++dy)
				for (int dx = 0; dx < from; ++dx)
					val = (val << 1) | og[(y + dy) * g->size + (x + dx)];
			val = rules[val];
			int ny = y/from * to;
			int nx = x/from * to;
			for (int dy = 0; dy < to; ++dy) {
				for (int dx = 0; dx < to; ++dx) {
					int b = (to - dy - 1) * to + (to - dx - 1); // bit location in val
					ng[(ny + dy) * nsize + (nx + dx)] = (val >> b) & 1;
				}
			}
		}
	}
	free(g->grid);
	g->size = nsize;
	g->grid = ng;
}

void print_grid(struct grid* g) {
	for (int y = 0; y < g->size; ++y) {
		for (int x = 0; x < g->size; ++x) {
			printf("%c", g->grid[y * g->size + x] ? '#' : '.');
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int rules2to3[1<<(2*2)];
	int rules3to4[1<<(3*3)];

	while (getline(&line, &len, stdin) != -1) {
		struct rule rule;
		read_rule(line, &rule);
		if (rule.insize == 2)
			permutate_rule2(&rule, rules2to3);
		else
			permutate_rule3(&rule, rules3to4);
	}
	free(line);

	// create new grid
	struct grid g;
	g.size = 3;
	g.grid = calloc(g.size * g.size, sizeof(bool));
	g.grid[1] = true;
	for (int ii = 5; ii <= 8; ++ii)
		g.grid[ii] = true;

	// print_grid(&g);

	for (int ii = 0; ii < 5; ++ii) {
		if (g.size % 2 == 0)
			apply_rule(&g, rules2to3, 2, 3);
		else
			apply_rule(&g, rules3to4, 3, 4);
		// printf("\n");
		// print_grid(&g);
	}
	int count = 0;
	for (int ii = 0; ii < g.size * g.size; ++ii)
		count += g.grid[ii] ? 1 : 0;
	printf("%d\n", count);

	free(g.grid);
	return 0;
}
