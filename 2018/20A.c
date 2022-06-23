#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// #define DEBUG

#define GRIDW 256
#define GRIDH (GRIDW)

#define POSSTACK_DEPTH 512

#define NR_DIRS 4
#define OPP_XOR 3
enum dir { // codes are s.t. dir XOR 3 = opposite dir
	E = 0,
	N = 1,
	S = 2,
	W = 3,
};
char* dirstr = "ENSW";
int dir2dx[] = {1, 0, 0, -1};
int dir2dy[] = {0, -1, 1, 0};

struct cell {
	bool door[4]; // e,n,w,s
	int  dist; // for distance finding
};

struct grid {
	int          gridw;
	int          gridh;
	struct cell* grid;

#ifdef DEBUG
	// min/max just for printing
	int          xmin;
	int          xmax;
	int          ymin;
	int          ymax;
#endif
};

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = w;
	g->gridh = h;
	g->grid = calloc(w * h, sizeof(struct cell));
#ifdef DEBUG
	g->xmin = INT_MAX;
	g->xmax = 0;
	g->ymin = INT_MAX;
	g->ymax = 0;
#endif
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

#ifdef DEBUG
void grid_update_minmax(struct grid* g, int x, int y) {
	g->xmin = x < g->xmin ? x : g->xmin;
	g->xmax = x > g->xmax ? x : g->xmax;
	g->ymin = y < g->ymin ? y : g->ymin;
	g->ymax = y > g->ymax ? y : g->ymax;
}
#endif

void is_door(struct grid* g, int x, int y, enum dir dir) {
	g->grid[y * g->gridw + x].door[dir] = true;
	// mark opposite side of door
#ifdef DEBUG
	grid_update_minmax(g, x, y);
#endif
	x += dir2dx[dir];
	y += dir2dy[dir];
#ifdef DEBUG
	grid_update_minmax(g, x, y);
#endif
	dir ^= OPP_XOR; // invert direction
	if (x < 0 || x >= g->gridw || y < 0 || y >= g->gridh)
		fprintf(stderr, "TODO: reserved grid too small!!\n");
	else
		g->grid[y * g->gridw + x].door[dir] = true;
}

#ifdef DEBUG
void print_grid(struct grid* g) {
	for (int y = g->ymin; y <= g->ymax; ++y) {
		for (int x = g->xmin; x <= g->xmax; ++x) {
			struct cell* c = &g->grid[y * g->gridw + x];
			printf("#%c", c->door[N] ? '-' : '#');
		}
		printf("#\n");
		for (int x = g->xmin; x <= g->xmax; ++x) {
			struct cell* c = &g->grid[y * g->gridw + x];
			printf("%c ", c->door[W] ? '|' : '#');
		}
		printf("#\n");
	}
	for (int x = g->xmin; x <= g->xmax; ++x)
		printf("##");
	printf("#\n");
}
#endif

struct pos {
	int x;
	int y;
};

struct pos_stack {
	struct pos* stack;
	int         depth;
	int         sp;
};

struct pos_stack* create_pos_stack(int depth) {
	struct pos_stack* ps = malloc(sizeof(struct pos_stack));
	ps->depth = depth;
	ps->stack = malloc(ps->depth * sizeof(struct pos));
	ps->sp = 0;
	return ps;
}

void destroy_pos_stack(struct pos_stack* ps) {
	if (ps) {
		free(ps->stack);
		free(ps);
	}
}

void pos_stack_push(struct pos_stack* ps, struct pos* pos) {
	if (ps->sp >= ps->depth)
		fprintf(stderr, "Pos stack overflow!\n");
	else
		ps->stack[ps->sp++] = *pos;
}

struct pos pos_stack_pop(struct pos_stack* ps) {
	struct pos pos = {0, 0};
	if (ps->sp == 0)
		fprintf(stderr, "Pos stack underflow!\n");
	else
		pos = ps->stack[--ps->sp];
	return pos;
}

struct pos pos_stack_peek(struct pos_stack* ps) {
	struct pos pos = {0, 0};
	if (ps->sp == 0)
		fprintf(stderr, "Pos stack: cannot peek from empty stack!\n");
	else
		pos = ps->stack[ps->sp - 1];
	return pos;
}

void print_pos_stack(struct pos_stack* ps) {
	for (int ii = ps->sp - 1; ii >= 0; --ii)
		printf("{%d, %d}\n", ps->stack[ii].x, ps->stack[ii].y);
}

/* ************ QUEUE ************** */
struct int_queue {
	unsigned int capacity; // max nr elements in q mem region
	unsigned int front;
	unsigned int back;
	int*         q; // memory for queue data
};

struct int_queue* create_int_queue() {
	struct int_queue* q = malloc(sizeof(struct int_queue));
	q->capacity = 1024;
	q->q = malloc(q->capacity * sizeof(int));
	q->front = 0;
	q->back = 0;
	return q;
}

void destroy_int_queue(struct int_queue* q) {
	if (q) {
		free(q->q);
		free(q);
	}
}

unsigned int int_queue_size(struct int_queue* q) {
	return (q->back + q->capacity - q->front) % q->capacity;
}

bool int_queue_isempty(struct int_queue* q) {
	return q->back == q->front;
}

int int_queue_dequeue(struct int_queue* q) {
	int e = q->q[q->front];
	q->front = (q->front + 1) % q->capacity;
	return e;
}

void int_queue_enqueue(struct int_queue* q, int e) {
	if (int_queue_size(q) + 1 >= q->capacity) { // enlarge capcity
		q->q = realloc(q->q, 2 * q->capacity * sizeof(int));
		if (q->back < q->front) {
			if (q->back > 0)
				memcpy(q->q + q->capacity, q->q, q->back * sizeof(int));
			q->back += q->capacity;
		}
		q->capacity *= 2;
	}
	q->q[q->back] = e;
	q->back = (q->back + 1) % q->capacity;
}
/* ************ END QUEUE ************** */

void calc_distances(struct grid* g, int x, int y) {
	int idx;
	struct int_queue* q = create_int_queue();

	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].dist = INT_MAX;

	idx = y * g->gridw + x;
	g->grid[idx].dist = 0;
	int_queue_enqueue(q, idx);
	while (!int_queue_isempty(q)) {
		idx = int_queue_dequeue(q);
		struct cell* c = &g->grid[idx];
		for (int dir = 0; dir < 4; ++dir) {
			if (c->door[dir]) {
				int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
				if (g->grid[idx].dist + 1 < g->grid[nidx].dist) {
					g->grid[nidx].dist = g->grid[idx].dist + 1;
					int_queue_enqueue(q, nidx);
				}
			}
		}
	}

	destroy_int_queue(q);
}

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid(GRIDW, GRIDH);
	struct pos_stack* pos_stack = create_pos_stack(POSSTACK_DEPTH);

	struct pos pos;
	pos.x = grid->gridw / 2;
	pos.y = grid->gridh / 2;
	struct pos startpos = pos;

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	// Step 1: follow the path, create grid content
	char* l = line + 1; // skip ^
	while (*l && *l != '$') {
		char* d = strchr(dirstr, *l);
		if (d) {
			enum dir dir = (enum dir)(d - dirstr);
			is_door(grid, pos.x, pos.y, dir);
			pos.x += dir2dx[dir];
			pos.y += dir2dy[dir];
		}
		else if (*l == '(') { // split in the road
			pos_stack_push(pos_stack, &pos);
		}
		else if (*l == '|') {
			pos = pos_stack_peek(pos_stack);
		}
		else if (*l == ')') {
			// algo fails if:
			// 1. after ) there is a direction
			// 2. not all paths in brackets have the same net movement
			pos_stack_pop(pos_stack); // THIS IS TRICKY, ONLY WORKS IN CONSTRAINED PUZZLES
			// or should I save return value back in pos? I think NOT doing that allows
			// net movement that is non-zero, as long as all paths in brackets have same net movement
		}
		++l;
	}
#ifdef DEBUG
	print_grid(grid);
#endif

	calc_distances(grid, startpos.x, startpos.y);
	// find largest distance
	int maxdist = 0;
	for (int ii = 0; ii < grid->gridw * grid->gridh; ++ii) {
		int dist = grid->grid[ii].dist;
		if (dist < INT_MAX)
			maxdist = dist > maxdist ? dist : maxdist;
	}
	printf("%d\n", maxdist);

	free(line);
	destroy_grid(grid);
	destroy_pos_stack(pos_stack);
	return 0;
}
