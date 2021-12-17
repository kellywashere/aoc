#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#define MAX_ELEMENTS 26
#define is_element(a) ((a)>='A' && (a)<='Z')

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "Could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

struct rule {
	// xy -> z
	char x;
	char y;
	char z;
	struct rule* next; // LL option
};

struct rule* process_rule_line(char* line) {
	struct rule* rule = mymalloc(sizeof(struct rule));
	//printf("%s", line);
	rule->x = *line;
	rule->y = *(line + 1);
	rule->z = *(line + 6);
	return rule;
}

int duplet_to_idx(char x, char y) {
	return (x - 'A') * MAX_ELEMENTS + (y - 'A');
}

void idx_to_duplet(int idx, char* x, char* y) {
	*x = idx / MAX_ELEMENTS + 'A';
	*y = idx % MAX_ELEMENTS + 'A';
}

uint64_t* process_template_line(char* line) {
	uint64_t* duplet_count = mymalloc(MAX_ELEMENTS * MAX_ELEMENTS * sizeof(uint64_t));
	int ii;
	for (ii = 0; ii < MAX_ELEMENTS * MAX_ELEMENTS; ++ii)
		duplet_count[ii] = 0;
	if (!is_element(line[0]) || !is_element(line[1]))
		return;
	for (ii = 1; is_element(line[ii]); ++ii)
		++duplet_count[duplet_to_idx(line[ii - 1], line[ii])];
	return duplet_count;
}

struct rule* find_rule(struct rule* rules, char x, char y) {
	while (rules) {
		if (rules->x == x && rules->y == y)
			return rules;
		rules = rules->next;
	}
	return NULL;
}

void apply_rules(uint64_t** duplet_count, struct rule* rules) {
	uint64_t* indc = *duplet_count;
	uint64_t* outdc = mymalloc(MAX_ELEMENTS * MAX_ELEMENTS * sizeof(uint64_t));
	int ii;
	for (ii = 0; ii < MAX_ELEMENTS * MAX_ELEMENTS; ++ii)
		outdc[ii] = 0;
	char x, y, z;
	for (ii = 0; ii < MAX_ELEMENTS * MAX_ELEMENTS; ++ii) {
		if (indc[ii]) {
			idx_to_duplet(ii, &x, &y);
			struct rule* r = find_rule(rules, x, y);
			if (r) {
				outdc[duplet_to_idx(x, r->z)] += indc[ii];
				outdc[duplet_to_idx(r->z, y)] += indc[ii];
			}
			else {
				outdc[duplet_to_idx(x, y)] += indc[ii];
			}
		}
	}
	free(indc);
	*duplet_count = outdc;
}

void count_elements(uint64_t* duplet_count, uint64_t* counts) {
	int ii;
	for (ii = 0; ii < MAX_ELEMENTS; ++ii)
		counts[ii] = 0;
	char x, y;
	for (ii = 0; ii < MAX_ELEMENTS * MAX_ELEMENTS; ++ii) {
		if (duplet_count[ii]) {
			idx_to_duplet(ii, &x, &y);
			counts[x - 'A'] += duplet_count[ii];
			counts[y - 'A'] += duplet_count[ii];
			// printf("Duplet %c%c: %" PRIu64 " times\n", x, y, duplet_count[ii]);
		}
	}
	// now we have counted all elements double, except for
	// first and last ones of template. But those can be found,
	// since these are only odd ones.
	for (ii = 0; ii < MAX_ELEMENTS; ++ii) {
		counts[ii] = (counts[ii] + 1)/2;
		if (counts[ii])
			printf("Element %c: %" PRIu64 " times\n", 'A' + ii, counts[ii]);
	}
}

int main(int argc, char* argv[]) {
	int iters = 40;
	if (argc > 1)
		iters = atoi(argv[1]);
	struct rule* rules = NULL;

	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	uint64_t* duplet_count = process_template_line(line);

	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		struct rule* r = process_rule_line(line);
		r->next = rules;
		rules = r;
	}
	free(line);

	int ii;
	for (ii = 0; ii < iters; ++ii)
		apply_rules(&duplet_count, rules);

	// count
	uint64_t counts[MAX_ELEMENTS];
	count_elements(duplet_count, counts);

	uint64_t mincount = 0;
	uint64_t maxcount = 0;
	for (ii = 0; ii < MAX_ELEMENTS; ++ii) {
		if (counts[ii] && (!mincount || counts[ii] < mincount))
			mincount = counts[ii];
		if (counts[ii] > maxcount)
			maxcount = counts[ii];
	}
	printf("%" PRIu64 "\n", maxcount - mincount);

	// clean up
	free(duplet_count);
	while (rules) {
		struct rule* r = rules;
		rules = r->next;
		free(r);
	}
	return 0;
}
