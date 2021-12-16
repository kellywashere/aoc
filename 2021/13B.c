#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_ATOMS 26

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

struct coord {
	int x;
	int y;
	struct coord* next;
};

struct coord* create_coord(int x, int y) {
	struct coord* c = mymalloc(sizeof(struct coord));
	c->x = x;
	c->y = y;
	c->next = NULL;
	return c;
}

int cmp_coords(struct coord* a, struct coord* b) {
	return (a->y == b->y) ? (a->x - b->x) : (a->y - b->y);
}

struct coord* mergesort_coords(struct coord* ll) {
	if (!ll || ll->next == NULL)
		return ll;
	// ll has at least 2 elements
	struct coord* e;
	struct coord* ll_split[2];
	ll_split[0] = ll_split[1] = NULL;
	// split list in two parts
	int lidx = 0;
	while (ll) {
		e = ll;
		ll = ll->next;
		e->next = ll_split[lidx];
		ll_split[lidx] = e;
		lidx = 1 - lidx;
	}
	// sort both parts using recursion
	ll_split[0] = mergesort_coords(ll_split[0]);
	ll_split[1] = mergesort_coords(ll_split[1]);
	// merge
	struct coord* tail = NULL;
	while (ll_split[0] != NULL && ll_split[1] != NULL) {
		lidx = cmp_coords(ll_split[0], ll_split[1]) >= 0 ? 1 : 0;
		e = ll_split[lidx];
		ll_split[lidx] = ll_split[lidx]->next;
		e->next = NULL;
		if (!ll)
			ll = e;
		if (tail)
			tail->next = e;
		tail = e;
	}
	if (ll_split[0])
		tail->next = ll_split[0];
	else
		tail->next = ll_split[1];
	return ll;
}

struct coord* remove_dups(struct coord* coords) {
	if (coords == NULL)
		return NULL;
	coords = mergesort_coords(coords);
	struct coord* e = coords;
	while (e->next) {
		struct coord* enext = e->next;
		if (e->x == enext->x && e->y == enext->y) {
			e->next = enext->next;
			free(enext);
		}
		else {
			e = e->next;
		}
	}
	return coords;
}

int coords_xmax(struct coord* coords) {
	if (coords == NULL)
		return 0;
	int x = coords->x;
	coords = coords->next;
	while (coords) {
		x = coords->x > x ? coords->x : x;
		coords = coords->next;
	}
	return x;
}

int coords_ymax(struct coord* coords) {
	if (coords == NULL)
		return 0;
	int y = coords->y;
	coords = coords->next;
	while (coords) {
		y = coords->y > y ? coords->y : y;
		coords = coords->next;
	}
	return y;
}

void process_dot_line(char* line, int* x, int* y) {
	*x = strtol(line, &line, 10);
	*y = strtol(line + 1, NULL, 10);
}

void print_coords(struct coord* coords) {
	while (coords) {
		printf("%d, %d\n", coords->x, coords->y);
		coords = coords->next;
	}
}

void fold_left(struct coord* coords, int x) {
	while (coords) {
		if (coords->x > x) {
			coords->x = 2 * x - coords->x;
			if (coords->x < 0)
				fprintf(stderr, "Negative coord after fold_up: x = %d\n", coords->x);
		}
		coords = coords->next;
	}
}

void fold_up(struct coord* coords, int y) {
	while (coords) {
		if (coords->y > y) {
			coords->y = 2 * y - coords->y;
			if (coords->y < 0)
				fprintf(stderr, "Negative coord after fold_up: y = %d\n", coords->y);
		}
		coords = coords->next;
	}
}

struct coord* process_fold_line(char* line, struct coord* coords) {
	int xy = atoi(line + 13);
	if (*(line + 11) == 'x')
		fold_left(coords, xy);
	else
		fold_up(coords, xy);
	coords = remove_dups(coords);
	return coords;
}

void print_dots(struct coord* dots) {
	int w = coords_xmax(dots) + 1;
	int h = coords_ymax(dots) + 1;
	char* grid = mymalloc(w * h * sizeof(char));
	memset(grid, ' ', w * h);
	while (dots) {
		grid[dots->y * w + dots->x] = '#';
		dots = dots->next;
	}
	int x, y;
	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			putchar(grid[y * w + x]);
			putchar(grid[y * w + x]);
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[]) {
	struct coord* dots = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int x, y;
		if (empty_line(line))
			break; // first section is over
		process_dot_line(line, &x, &y);
		struct coord* c = create_coord(x, y);
		c->next = dots;
		dots = c;
	}
	while (getline(&line, &len, stdin) != -1) {
		int xy;
		if (empty_line(line))
			continue; // first section is over
		dots = process_fold_line(line, dots);
	}
	print_dots(dots);
	free(line);

	// clean up
	while (dots) {
		struct coord* c = dots;
		dots = c->next;
		free(c);
	}
	return 0;
}
