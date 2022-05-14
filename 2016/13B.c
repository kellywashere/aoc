#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

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

int popcount(int x) {
	int c = 0;
	while (x) {
		c += x & 1;
		x >>= 1;
	}
	return c;
}

bool is_wall(int x, int y, int key) {
	int n = x*x + 3*x + 2*x*y + y + y*y;
	n += key;
	return popcount(n) % 2 == 1;
}

void draw_grid(int w, int h, int key, int targetx, int targety) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			bool is_target = x == targetx && y == targety;
			printf("%c", is_target ? 'X' : is_wall(x, y, key) ? '#' : '.');
		}
		printf("\n");
	}
}

enum dir {
	STATIONARY,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NR_DIRS
};

void dir_to_dxdy(enum dir dir, int gridw, int* dx, int* dy) {
	*dx = 0;
	*dy = 0;
	switch(dir) {
		case UP:
			*dy = -1;
			break;
		case DOWN:
			*dy = 1;
			break;
		case LEFT:
			*dx = -1;
			break;
		case RIGHT:
			*dx = 1;
			break;
		default:
	}
}

int count_reachable_locations(int startx, int starty, int steps, int key) {
	int idx, x, y, dx, dy;
	struct int_queue* intq = create_int_queue();
	// calc limits
	int gridw = startx + steps + 1;
	int gridh = starty + steps + 1;
	bool* grid = malloc(gridw * gridh * sizeof(bool));
	int* count = malloc(gridw * gridh * sizeof(int));
	for (x = 0; x < gridw; ++x) {
		for (y = 0; y < gridh; ++y) {
			idx = y * gridw + x;
			grid[idx] = is_wall(x, y, key);
			count[idx] = INT_MAX;
		}
	}

	// bfs
	idx = starty * gridw + startx;
	count[idx] = 0;
	int_queue_enqueue(intq, idx);
	int total = 0;
	while (!int_queue_isempty(intq)) {
		++total;
		idx = int_queue_dequeue(intq);
		x = idx % gridw;
		y = idx / gridw;
		int s = count[idx];
		if (s < steps) {
			for (enum dir dir = UP; dir < NR_DIRS; ++dir) {
				dir_to_dxdy(dir, gridw, &dx, &dy);
				int nx = x + dx;
				int ny = y + dy;
				int nidx = ny * gridw + nx;
				if (nx < 0 || nx >= gridw || ny < 0 || ny >= gridh || grid[nidx])
					continue;
				if (s + 1 < count[nidx]) {
					count[nidx] = s + 1;
					int_queue_enqueue(intq, nidx);
				}
			}
		}
	}

	free(grid);
	free(count);
	destroy_int_queue(intq);
	return total;
}

int main(int argc, char* argv[]) {
	// draw_grid(10, 10, 10, 7, 4); // test example
	// draw_grid(50, 50, 1364, 31, 39);

	int count = count_reachable_locations(1, 1, 50, 1364);
	printf("%u\n", count);

	return 0;
}
