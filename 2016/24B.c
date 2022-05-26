#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

#define EMPTY 100
#define WALL  101

// From projecteuler, problem 041.c
bool next_perm(int a[], int len) {
	// returns false when no next permutation exists
	// step 1: from right to left, find first element higher than its right neighbor: pivot = a[idxPivot]
	// step 2: sort elements to right of idxPivot (invert order in this case!)
	// step 3: from idxPivot upward, find first element > pivot: idxSwap
	// step 4: swap pivot with a[idxSwap]
	int idxPivot;
	for (idxPivot = len - 2; idxPivot >= 0; --idxPivot) {
		if (a[idxPivot] < a[idxPivot + 1]) {
			// invert order of elements idxPivot+1 ... len-1
			int t;
			int ii = idxPivot + 1;
			int jj = len - 1;
			while (jj > ii) {
				t = a[jj];
				a[jj--] = a[ii];
				a[ii++] = t;
			}
			// from idxPivot upward, find first element > pivot: ii
			for (ii = idxPivot + 1; a[ii] < a[idxPivot]; ++ii);
			// swap a[ii] with a[idxPivot]
			t = a[ii];
			a[ii] = a[idxPivot];
			a[idxPivot] = t;
			break;
		}
	}
	return (idxPivot >= 0);
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

bool int_queue_contains(struct int_queue* q, int e) {
	for (int ii = q->front; ii != q->back; ii = (ii + 1) % q->capacity)
		if (q->q[ii] == e)
			return true;
	return false;
}
/* ************ END QUEUE ************** */


struct grid {
	int  gridw;
	int  gridh;
	int  rowcapacity; // max nr of rows reserved in mem
	int* grid;
	int  maxlocnr;    // highest target digit
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	g->maxlocnr = 0;
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

void grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (!isspace(line[lw]))
		++lw;
	if (g->gridw > 0 && g->gridw != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return;
	}
	if (!g->grid) {
		g->gridw = lw;
		g->gridh = 0;
		g->rowcapacity = lw; // assume square first
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(int));
		for (int ii = 0; ii < g->gridw * g->rowcapacity; ++ii)
			g->grid[ii] = EMPTY;
	}
	if (g->gridh >= g->rowcapacity) {
		g->grid = realloc(g->grid, g->gridw * 2 * g->rowcapacity * sizeof(int));
		for (int ii = g->gridw * g->rowcapacity; ii < g->gridw * 2 * g->rowcapacity; ++ii)
			g->grid[ii] = EMPTY;
		g->rowcapacity *= 2;
	}
	for (int col = 0; col < g->gridw; ++col) {
		g->grid[col + g->gridh * g->gridw] = line[col] == '#' ? WALL :
		                                     line[col] == '.' ? EMPTY :
		                                     line[col] - '0';
		if (line[col] >= '0' && line[col] <= '9' && (line[col] - '0' > g->maxlocnr))
			g->maxlocnr = line[col] - '0';
	}
	++g->gridh;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = x + y * g->gridw;
			printf("%c", g->grid[idx] == WALL ? '#' : g->grid[idx] == EMPTY ? '.' : g->grid[idx] + '0');
		}
		printf("\n");
	}
}

bool find_target(struct grid* g, int target, int* row, int* col) {
	for (int r = 0; r < g->gridh; ++r) {
		for (int c = 0; c < g->gridw; ++c) {
			if (g->grid[c + g->gridw * r] == target) {
				*row = r;
				*col = c;
				return true;
			}
		}
	}
	return false;
}

int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

void find_path_lens_from(struct grid* g, int from, int* distances) {
	// distances is an array of ints, with capacity g->maxlocnr + 1
	int rowfrom = 0;
	int colfrom = 0;
	int idx;
	find_target(g, from, &rowfrom, &colfrom);
	// bfs until all found
	int found = 0;
	struct int_queue* q = create_int_queue();
	int* dist = malloc(g->gridw * g->gridh * sizeof(int));
	for (idx = 0; idx < g->gridw * g->gridh; ++idx)
		dist[idx] = INT_MAX; // also indicates not visited
	idx = colfrom + rowfrom * g->gridw;

	int_queue_enqueue(q, idx);
	dist[idx] = 0;
	while (found < g->maxlocnr + 1 && !int_queue_isempty(q)) {
		idx = int_queue_dequeue(q);
		int x = idx % g->gridw;
		int y = idx / g->gridw;
		if (g->grid[idx] <= g->maxlocnr) { // current is target
			++found;
			// printf("Distance to %d: %d\n", g->grid[idx], dist[idx]);
			distances[g->grid[idx]] = dist[idx];
		}
		for (int ii = 0; ii < 4; ++ii) {
			int xn = x + dx[ii];
			int yn = y + dy[ii];
			int idxn = xn + yn * g->gridw;
			if (xn < 0 || xn >= g->gridw || yn < 0 || yn >= g->gridh || g->grid[idxn] == WALL)
				continue;
			if (dist[idx] + 1 < dist[idxn]) {
				dist[idxn] = dist[idx] + 1;
				int_queue_enqueue(q, idxn);
			}
		}
	}

	destroy_int_queue(q);
	free(dist);
}

int find_shortest_route(int* distances, int matrixwh) {
	// brute force: permutate over all options!
	int* order = malloc(matrixwh * (sizeof(int) + 1));
	for (int ii = 0; ii < matrixwh; ++ii)
		order[ii] = ii;
	order[matrixwh] = 0; // return to start (24B)
	int shortest_dist = INT_MAX;
	do {
		// calc distance according to current visiting order
		int dist = 0;
		for (int to_idx = 1; to_idx < matrixwh + 1; ++to_idx) {
			int from = order[to_idx - 1];
			int to = order[to_idx];
			dist += distances[from * matrixwh + to];
		}
		shortest_dist = dist < shortest_dist ? dist : shortest_dist;
	} while (next_perm(order + 1, matrixwh - 1)); // do not permute starting point and ending point
	free(order);
	return shortest_dist;
}


int main(int argc, char* argv[]) {
	struct grid* g = create_grid();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid_add_line(g, line);
	}
	// show_grid(g);

	int* distances = malloc((g->maxlocnr + 1) * (g->maxlocnr + 1) * sizeof(int));
	for (int from = 0; from <= g->maxlocnr; ++from) // doing double work (symmetric matrix) but who cares?
		find_path_lens_from(g, from, distances + from * (g->maxlocnr + 1));

	/*
	// show distance matrix
	for (int from = 0; from <= g->maxlocnr; ++from) {
		for (int to = 0; to <= g->maxlocnr; ++to) {
			int idx = from * (g->maxlocnr + 1) + to;
			printf("%5d", distances[idx]);
		}
		printf("\n");
	}
	*/

	int pathlen = find_shortest_route(distances, g->maxlocnr + 1);
	printf("%d\n", pathlen);

	destroy_grid(g);
	free(line);
	free(distances);
	return 0;
}
