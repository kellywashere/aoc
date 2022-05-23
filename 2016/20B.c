#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef unsigned long long int ipaddress;

#define MAX_IPADDRESS 0x0ffffffffULL

struct range {
	ipaddress from;
	ipaddress to; // inclusive
};

struct rangelist {
	struct range* ranges;
	int           capacity;
	int           size;
};

struct rangelist* create_rangelist() {
	struct rangelist* rl = malloc(sizeof(struct rangelist));
	rl->size = 0;
	rl->capacity = 512;
	rl->ranges = malloc(rl->capacity * sizeof(struct range));
	return rl;
}

void destroy_rangelist(struct rangelist* rl) {
	if (rl) {
		free(rl->ranges);
		free(rl);
	}
}

void rangelist_add(struct rangelist* rl, struct range* r) {
	if (rl->size == rl->capacity) {
		rl->capacity *= 2;
		rl->ranges = realloc(rl->ranges, rl->capacity * sizeof(struct range));
	}
	rl->ranges[rl->size].from = r->from;
	rl->ranges[rl->size].to = r->to;
	++rl->size;
}

static int cmp_ranges(const void* a, const void* b) {
	struct range* ra = (struct range*)a;
	struct range* rb = (struct range*)b;
	return ra->from > rb->from ? 1 : (ra->from == ra->to ? 0 : -1);
}

void rangelist_sort(struct rangelist* rl) {
	qsort(rl->ranges, rl->size, sizeof(struct range), cmp_ranges);
}

unsigned long long int rangelist_countfree(struct rangelist* rl, ipaddress* to) {
	rangelist_sort(rl);
	unsigned long long int count = 0;
	if (rl->ranges[0].from > 0)
		count = rl->ranges[0].from - 1;
	*to = rl->ranges[0].to;
	for (int ii = 1; ii < rl->size; ++ii) {
		if (rl->ranges[ii].from > *to + 1)
			count += rl->ranges[ii].from - *to - 1;
		*to = rl->ranges[ii].to > *to ? rl->ranges[ii].to : *to;
	}
	return count;
}


void read_range(const char* l, struct range* r) {
	r->from = 0;
	r->to = 0;
	while (*l && (*l < '0' || *l > '9'))
		++l;
	while (*l >= '0' && *l <= '9') {
		r->from = r->from * 10 + *l - '0';
		++l;
	}
	while (*l && (*l < '0' || *l > '9'))
		++l;
	while (*l >= '0' && *l <= '9') {
		r->to = r->to * 10 + *l - '0';
		++l;
	}
}

void print_range(struct range* r) {
	printf("%llu ... %llu\n", r->from, r->to);
}

int main(int argc, char* argv[]) {
	struct rangelist* rl = create_rangelist();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		struct range r;
		read_range(line, &r);
		rangelist_add(rl, &r);
	}
	ipaddress to;
	ipaddress count = rangelist_countfree(rl, &to);
	count += MAX_IPADDRESS - to;
	printf("%llu\n", count);

	free(line);
	destroy_rangelist(rl);
	return 0;
}
