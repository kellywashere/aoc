#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#define MAX_SEEDS     32
#define MAX_MAP_SIZE 128
#define NR_MAPLISTS    7

typedef unsigned int uint;

bool verbose = false;

struct range {
	uint from;
	uint to; // inclusive
};

struct range_list {
	struct range range;
	struct range_list* next;
};

struct map {
	uint         dest;
	struct range src;
};

struct map_list {
	struct map map;

	struct map_list* next;
};

struct range_list* merge_range_lists(struct range_list* front, struct range_list* back) {
	if (!front) return back;
	if (!back) return front;
	struct range_list* tail = front;
	while (tail->next)
		tail = tail->next;
	tail->next = back;
	return front;
}

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

struct map_list* read_maplist(void) {
	struct map_list* ml = NULL;
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
			struct map_list* e = malloc(sizeof(struct map_list));
			e->map.dest = dest;
			e->map.src.from = src;
			e->map.src.to = src + rangelen - 1;
			e->next = ml;
			ml = e;
		}
	}
	free(line);
	return ml;
}

void print_range_list(struct range_list* rl) {
	if (rl == NULL)
		printf("EMPTY RANGE LIST\n");
	for (; rl; rl = rl->next)
		printf("%u...%u\n", rl->range.from, rl->range.to);
}

void print_map(struct map* m) {
	uint f = m->src.from;
	uint t = m->src.to;
	uint d = m->dest;
	printf("[%u...%u] ", f, t);
	if (d >= f)
		printf("+%u ", d - f);
	else
		printf("-%u ", f - d);
}

void print_maplist(struct map_list* ml) {
	while (ml) {
		print_map(&ml->map);
		printf("\n");
		ml = ml->next;
	}
	printf("\n");
}

bool ranges_overlap(struct range* r1, struct range* r2) {
	return !(r1->to < r2->from || r2->to < r1->from);
}

struct range_list* create_rangelist_el(uint from, uint to) {
	struct range_list* el = malloc(sizeof(struct range_list));
	el->range.from = from;
	el->range.to = to;
	el->next = NULL;
	return el;
}

void apply_map_to_rangelist(struct map* m, struct range_list* rlin,
		struct range_list** unmapped, struct range_list** mapped) {
	// applies single map to range list
	// returns two pointers to struct range_list*: unmapped part and mapped part
	// original list rlin is destroyed

	if (verbose) {
		printf("Applying map: ");
		print_map(m);
		printf("to range list:\n");
		print_range_list(rlin);
	}

	struct range* mr = &m->src;
	*unmapped = NULL;
	*mapped = NULL;
	while (rlin) {
		struct range_list* el = rlin;
		rlin = rlin->next;

		uint from = el->range.from;
		uint to = el->range.to;
		free(el);

		uint ovl_from = MAX(from, mr->from);
		uint ovl_to = MIN(to, mr->to);
	
		if (ovl_to < ovl_from) { // no overlap
			*unmapped = merge_range_lists(create_rangelist_el(from, to), *unmapped);
		}
		else { // there is overlap
			uint dest_from = ovl_from - mr->from + m->dest;
			uint dest_to = dest_from + (ovl_to - ovl_from);
			*mapped = merge_range_lists(create_rangelist_el(dest_from, dest_to), *mapped);
			if (to > ovl_to) {
				*unmapped = merge_range_lists(create_rangelist_el(ovl_to + 1, to), *unmapped);
			}
			if (from < ovl_from) {
				*unmapped = merge_range_lists(create_rangelist_el(from, ovl_from - 1), *unmapped);
			}
		}
	}
	if (verbose) {
		printf("Unmapped:\n");
		print_range_list(*unmapped);
		printf("Mapped:\n");
		print_range_list(*mapped);
	}
}

struct range_list* apply_maplist_to_range_list(struct map_list* ml, struct range_list* rlin) {
	struct range_list* rl_out = NULL;
	for (struct map_list* mlel = ml; mlel; mlel = mlel->next) {
		struct range_list* unmapped = NULL;
		struct range_list* mapped = NULL;
		apply_map_to_rangelist(&mlel->map, rlin, &unmapped, &mapped);
		rlin = unmapped;
		rl_out = merge_range_lists(mapped, rl_out);
	}
	rl_out = merge_range_lists(rl_out, rlin); // recombine mapped and unmapped
	return rl_out;
}

int main(int argc, char* argv[]) {
	struct range seed_ranges[MAX_SEEDS];
	int nr_seed_ranges = 0;

	char *line = NULL;
	size_t len = 0;

	// read seeds
	getline(&line, &len, stdin);
	char* l = line;
	uint from;
	while (read_uint(&l, &from)) {
		uint rangelen;
		read_uint(&l, &rangelen);
		seed_ranges[nr_seed_ranges].from = from;
		seed_ranges[nr_seed_ranges].to = from + rangelen - 1;
		++nr_seed_ranges;
		// printf("\nSeed range: %u...%u\n", from, from + rangelen - 1);
	}
	// for (int ii = 0; ii < nr_seeds; ++ii)
	// 	printf("%u\n", seeds[ii]);
	free(line);

	struct map_list* maps[NR_MAPLISTS];
	for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
		maps[ii] = read_maplist();
	}

	uint minloc = UINT_MAX;
	for (int sidx = 0; sidx < nr_seed_ranges; ++sidx) {
		uint f = seed_ranges[sidx].from;
		uint t = seed_ranges[sidx].to;
		struct range_list* rl = create_rangelist_el(f, t);
		for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
			rl = apply_maplist_to_range_list(maps[ii], rl);
		}
		// find minloc
		for (struct range_list* el = rl; el; el = el->next)
			minloc = el->range.from < minloc ? el->range.from : minloc;
		// print and delete
		while (rl) {
			struct range_list* e = rl;
			rl = rl->next;
			free(e);
		}
	}
	printf("%u\n", minloc);
	
	// clean-up
	for (int ii = 0; ii < NR_MAPLISTS; ++ii) {
		while (maps[ii]) {
			struct map_list* e = maps[ii];
			maps[ii] = e->next;
			free(e);
		}
	}

	return 0;
}
