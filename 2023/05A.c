#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define MAX_SEEDS     32
#define MAX_MAP_SIZE 128
#define NR_MAPLISTS    7

typedef unsigned int uint;

struct range {
	uint from;
	uint to; // inclusive
};

struct map {
	uint         dest;
	struct range src;

	struct map* next;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_uint(char** pLine, uint* x) {
	uint num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	*x = num;
	return true;
}

struct map* read_maplist(void) {
	struct map* map = NULL;
	// find header
	char *line = NULL;
	size_t len = 0;
	bool header_found = false;
	while (!header_found && getline(&line, &len, stdin) != -1) {
		header_found = strstr(line, "map:") != NULL;
	}
	bool done = false;
	while (!done && getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			done = true;
		else {
			char* l = line;
			uint dest, src, rangelen;
			read_uint(&l, &dest);
			read_uint(&l, &src);
			read_uint(&l, &rangelen);
			struct map* e = malloc(sizeof(struct map));
			e->dest = dest;
			e->src.from = src;
			e->src.to = src + rangelen - 1;
			e->next = map;
			map = e;
		}
	}
	free(line);
	return map;
}

void print_maplist(struct map* m) {
	while (m) {
		printf("%u ", m->dest);
		printf("%u ", m->src.from);
		printf("%u\n", m->src.to - m->src.from + 1);
		m = m->next;
	}
	printf("\n");
}

uint map_nr(struct map* m, uint x) {
	while (m) {
		if (x >= m->src.from && x <= m->src.to) {
			// printf("Nr %u found in map: ", x);
			// printf("%u ", ml->map[ii].dest);
			// printf("%u ", ml->map[ii].src);
			// printf("%u\n", ml->map[ii].len);
			return x - m->src.from + m->dest;
		}
		m = m->next;
	}
	return x;
}

int main(int argc, char* argv[]) {
	uint seeds[MAX_SEEDS];
	int nr_seeds = 0;

	char *line = NULL;
	size_t len = 0;

	// read seeds
	getline(&line, &len, stdin);
	char* l = line;
	while (read_uint(&l, seeds + nr_seeds))
		++nr_seeds;
	// for (int ii = 0; ii < nr_seeds; ++ii)
	// 	printf("%u\n", seeds[ii]);
	free(line);

	struct map* maps[NR_MAPLISTS];
	for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
		maps[ii] = read_maplist();
		// print_maplist(maps[ii]);
	}

	uint minloc = UINT_MAX;
	for (int sidx = 0; sidx < nr_seeds; ++sidx) {
		uint x = seeds[sidx];
		for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
			// printf("%u --> ", x);
			x = map_nr(maps[ii], x);
			// printf("%u\n", x);
		}
		minloc = x < minloc ? x : minloc;
	}
	printf("%u\n", minloc);
	
	// clean-up
	for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
		while (maps[ii]) {
			struct map* e = maps[ii];
			maps[ii] = e->next;
			free(e);
		}
	}

	return 0;
}
