#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

uint64_t read_uint64(char** pLine) {
	uint64_t num = 0;
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
	return num;
}

struct memlist {
	int             addr;
	uint64_t        val;
	struct memlist* next; // LL
};

struct mask {
	uint64_t zero_mask; // ANDed with value
	uint64_t one_mask;  // ORed with value
};

struct memlist* insert(struct memlist* list, struct memlist* e) {
	// insert s.t. addresses are increasing
	if (list == NULL)
		return e;
	// find first element el that has el->addr >= e->addr
	struct memlist* el = list;
	struct memlist* el_prev = NULL;
	while (el && el->addr < e->addr) {
		el_prev = el;
		el = el->next;
	}
	if (el && el->addr == e->addr)
		el->val = e->val; // overwrite value
	else { // el->addr > e->addr || el == NULL
		e->next = el;
		if (el_prev)
			el_prev->next = e;
		else
			list = e;
	}
	return list;
}

void reset_mask(struct mask* mask) {
	mask->zero_mask = ~0;
	mask->one_mask = 0;
}

void set_mask(struct mask* mask, char* l) {
	reset_mask(mask);
	char* start = l;
	// find first valid char
	while (*l && *l != 'X' && *l != '0' && *l != '1')
		++l;
	// find first non-valid char
	while (*l == 'X' || *l == '0' || *l == '1')
		++l;
	// now go backwards
	--l;
	uint64_t b = 1; // bit pos
	while (l >= start && (*l == 'X' || *l == '0' || *l == '1')) {
		if (*l == '0')
			mask->zero_mask &= ~b;
		else if (*l == '1')
			mask->one_mask |= b;
		b <<= 1;
		--l;
	}
}

uint64_t apply_mask(struct mask* mask, uint64_t x) {
	x &= mask->zero_mask;
	x |= mask->one_mask;
	return x;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct memlist* list = NULL;
	struct mask mask;
	reset_mask(&mask);

	while (getline(&line, &len, stdin) != -1) {
		if (strstr(line, "mask"))
			set_mask(&mask, line + 7);
		else if (strstr(line, "mem")) {
			char* l = line;
			struct memlist* e = malloc(sizeof(struct memlist));
			e->addr = read_uint64(&l);
			e->val  = apply_mask(&mask, read_uint64(&l));
			e->next = NULL;

			list = insert(list, e);
		}
	}

	uint64_t sum = 0;
	for (struct memlist* e = list; e; e = e->next)
		sum += e->val;
	printf("%"PRIu64"\n", sum);

	while (list) {
		struct memlist* e = list;
		list = list->next;
		free(e);
	}
	free(line);
	return 0;
}
