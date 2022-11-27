#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

struct range {
	int from;
	int to; // inclusive
};

struct ranges {
	struct range   range[2]; // each range line contains 2 ranges
	struct ranges* next; // LL
};

void print_range(struct range* r) {
	printf("%d-%d", r->from, r->to);
}

void print_ranges(struct ranges* r) {
	print_range(&r->range[0]);
	printf(" or ");
	print_range(&r->range[1]);
	printf("\n");
}

bool is_valid(int n, struct ranges* r) {
	while (r) {
		for (int ii = 0; ii < 2; ++ii)
			if (n >= r->range[ii].from && n <= r->range[ii].to)
				return true;
		r = r->next;
	}
	return false;
}

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

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
	*pLine = line;
	*x = num;
	return true;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct ranges* ranges = NULL;
	// read ranges
	while (getline(&line, &len, stdin) != -1 && !empty_line(line)) {;
		char* l = line;
		struct ranges* r = malloc(sizeof(struct ranges));
		r->next = ranges;
		ranges = r;
		read_int(&l, &r->range[0].from);
		read_int(&l, &r->range[0].to);
		read_int(&l, &r->range[1].from);
		read_int(&l, &r->range[1].to);
	}
	// find nearby tickets
	while (getline(&line, &len, stdin) != -1 && !strstr(line, "nearby"))
		;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int n;
		while (read_int(&l, &n)) {
			if (!is_valid(n, ranges))
				sum += n;
		}
	}
	printf("%d\n", sum);

	while (ranges) {
		struct ranges* r = ranges;
		ranges = ranges->next;
		free(r);
	}
	free(line);
	return 0;
}
