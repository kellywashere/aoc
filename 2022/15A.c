#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(x) ((x)<0?(-(x)):(x))

#define y_problem 2000000

struct range {
	int           from;
	int           to;
	struct range* next; // allow LL
};

void show_rangelist(struct range* rl) {
	for (struct range* r = rl; r; r = r->next)
		printf("%d..%d\n", r->from, r->to);
}

bool ranges_overlap(int from1, int to1, int from2, int to2) {
	return !(to1 < from2 || to2 < from1);
}

struct range* rangelist_add(struct range* rl, int from, int to) {
	// adds new range, keeps things sorted by from field, and merges ranges
	if (from > to) {
		int t = from;
		from = to;
		to = t;
	}

	// create new range
	struct range* r = malloc(sizeof(struct range));
	r->from = from;
	r->to = to;

	// find position to insert in list
	struct range* before = (rl && rl->from < from) ? rl : NULL;
	while (before && before->next && before->next->from < from)
		before = before->next;

	if (before) {
		r->next = before->next;
		before->next = r;
	}
	else {
		r->next = rl;
		rl = r;
	}

	// merge elements wherever possible
	r = rl;
	while (r && r->next) {
		if (ranges_overlap(r->from, r->to, r->next->from, r->next->to)) {
			r->to = MAX(r->to, r->next->to);
			struct range* r_free = r->next;
			r->next = r->next->next;
			free(r_free);
		}
		else
			r = r->next;
	}

	return rl;
}

struct range* rangelist_sub(struct range* rl, int from, int to) {
	// cuts out portion of range

	struct range* prev = NULL;
	struct range* r = rl;
	while (r && r->from <= to) {
		struct range* r_free = NULL;
		// four cases
		// case 1: from-to completely covers r --> remove r
		if (from <= r->from && to >= r->to) {
			if (prev)
				prev->next = r->next;
			else // change head of list
				rl = r->next;
			r_free = r;
			r = r->next;
			printf("Free willy\n");
			free(r_free);
		}
		// case 2: from-to cuts r in two parts
		else if (from > r->from && to < r->to) {
			struct range* r2 = malloc(sizeof(struct range));
			r->to = from - 1;
			r2->from = to + 1;
			r2->to = r->to;
			r2->next = r->next;
			r->next = r2;
			break; // nothing left to do
		}
		// case 3: from-to cuts off lower part of r
		else if (from <= r->from && to < r->to) {
			r->from = to + 1;
			break; // nothing left to do
		}
		// case 3: from-to cuts off upper part of r
		else if (from > r->from && to >= r->to) {
			r->to = from - 1;
			r = r->next;
		}
		else
			r = r->next;
	}

	return rl;
}

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	if (isdigit(*line) && line > *pLine && *(line - 1) == '-')
		isneg = true;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? (-num) : num;
}

int main(int argc, char* argv[]) {
	struct range* rl = NULL; // range list where beacons cannot be

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int sx = read_int(&l);
		int sy = read_int(&l);
		int bx = read_int(&l);
		int by = read_int(&l);
		int dist = ABS(sx - bx) + ABS(sy - by);

		// calc range where (another) beacon cannot be for y = y_problem
		int x1 = sx + ABS(sy - y_problem) - dist;
		int x2 = sx + dist - ABS(sy - y_problem);
		if (x2 >= x1) {
			rl = rangelist_add(rl, x1, x2);
			if (by == y_problem)
				rl = rangelist_sub(rl, bx, bx);
			// printf("sensor@%d,%d: %d ... %d\n", sx, sy, x1, x2);
			// printf("Range list:\n");
			// show_rangelist(rl);
		}
	}
	free(line);

	show_rangelist(rl);
	// count total size of range list
	int size = 0;
	for (struct range* r = rl; r; r = r->next)
		size += r->to - r->from + 1;
	printf("%d\n", size);

	while (rl) {
		struct range* r = rl;
		rl = rl->next;
		free(r);
	}

	return 0;
}
