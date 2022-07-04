#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(a) ((a)<0?(-(a)):(a))

// all points in input file have coords in range [-8..8] (17 options per dimension)
#define M_ABS  8
#define GRIDSZ (2*M_ABS + 1)
#define N_GRID_POINTS (GRIDSZ*GRIDSZ*GRIDSZ*GRIDSZ)

#define ORTHOPLEX_SZ 129

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

bool find_unassigned_idx(int* grid, int* idx) {
	for (int ii = 0; ii < N_GRID_POINTS; ++ii) {
		if (grid[ii] == -1) {
			*idx = ii;
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {
	// generate all points in 4D orthoplex
	int dx[ORTHOPLEX_SZ][4];
	int idx = 0;
	for (int x = -3; x <= 3; ++x) {
		int d1 = ABS(x);
		for (int y = -3 + d1; y <= 3 - d1; ++y) {
			int d2 = d1 + ABS(y);
			for (int z = -3 + d2; z <= 3 - d2; ++z) {
				int d3 = d2 + ABS(z);
				for (int t = -3 + d3; t <= 3 - d3; ++t) {
					dx[idx][0] = x;
					dx[idx][1] = y;
					dx[idx][2] = z;
					dx[idx][3] = t;
					++idx;
				}
			}
		}
	}

	int grid[N_GRID_POINTS];
	for (int ii = 0; ii < N_GRID_POINTS; ++ii)
		grid[ii] = 0; // 0: not an input coordinate

	int x, y, z, t;
	while (scanf("%d,%d,%d,%d", &x, &y, &z, &t) == 4) {
		int idx = (x + M_ABS) + (y + M_ABS) * GRIDSZ + (z + M_ABS) * GRIDSZ * GRIDSZ + (t + M_ABS) * GRIDSZ * GRIDSZ * GRIDSZ;
		grid[idx] = -1; // -1: unassigned point in grid cell
	}

	struct int_queue* q = create_int_queue();

	int group_id = 1;
	while (find_unassigned_idx(grid, &idx)) {
		// start things off by putting unassigned cell in queue
		int_queue_enqueue(q, idx);

		while (!int_queue_isempty(q)) {
			idx = int_queue_dequeue(q);
			grid[idx] = group_id;

			int x[4];
			int xn[4]; // `neighbor`
			for (int ii = 0; ii < 4; ++ii) {
				x[ii] = idx % GRIDSZ;
				idx /= GRIDSZ;
			}
			// go over orthoplex for dx[]
			for (int ii = 0; ii < ORTHOPLEX_SZ; ++ii) {
				bool in_bounds = true;
				for (int dim = 0; dim < 4; ++dim) {
					xn[dim] = x[dim] + dx[ii][dim];
					in_bounds = in_bounds && xn[dim] >= 0 && xn[dim] < GRIDSZ;
				}
				if (in_bounds) {
					idx = xn[0] + xn[1] * GRIDSZ + xn[2] * GRIDSZ * GRIDSZ + xn[3] * GRIDSZ * GRIDSZ * GRIDSZ;
					if (grid[idx] == -1)
						int_queue_enqueue(q, idx);
				}
			}
		}
		++group_id;
	}
	printf("%d\n", group_id - 1);

	destroy_int_queue(q);
	return 0;
}
