#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

/* ************ QUEUE ************** */
struct int_queue {
	unsigned int capacity; // max nr elements in q mem region
	unsigned int front;
	unsigned int back;
	int*         q; // memory for queue data
};

struct int_queue* create_int_queue(void) {
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


enum type {
	EMPTY,
	WALL,
	DOOR,
	KEY
};

struct cell {
	enum type type;
	int       code; // key, door
	int       cost; // additional cost due to key move in dead end hallways
	int       helper; // general algo helper
};

int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, -1, 0, 1};

struct grid {
	int          gridw;
	int          gridh;
	int          rowcapacity; // max nr of rows reserved in mem
	struct cell* grid;
	int          posx;
	int          posy;
	int          totcost;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	g->posx = -1;
	g->posy = -1;
	g->totcost = 0;
	return g;
}

void destroy_grid(struct grid* g) {
	// does not destroy entities in grid
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (!isspace(line[lw]))
		++lw;
	if (g->gridw > 0 && g->gridw != lw) {
		fprintf(stderr, "Line width inconsistency\n");
		return false;
	}
	if (!g->grid) {
		g->gridw = lw;
		g->gridh = 0;
		g->rowcapacity = lw; // assume square
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(struct cell));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(struct cell));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx].type = line[col] == '#' ? WALL :
		                    islower(line[col]) ? KEY :
		                    isupper(line[col]) ? DOOR :
		                    EMPTY;
		if (g->grid[idx].type == KEY)
			g->grid[idx].code = line[col] - 'a';
		else if (g->grid[idx].type == DOOR)
			g->grid[idx].code = line[col] - 'A';
		g->grid[idx].cost = 0;
		g->grid[idx].helper = -1;
		if (line[col] == '@') {
			g->posx = col;
			g->posy = g->gridh;
		}
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;

			char c = g->grid[idx].type == WALL ? '#' :
			         g->grid[idx].type == KEY ? g->grid[idx].code + 'a' :
			         g->grid[idx].type == DOOR ? g->grid[idx].code + 'A':
			         (x == g->posx && y == g->posy) ? '@' :
			         '.';
			if (c == '.' && g->grid[idx].helper >= 0)
				c = (g->grid[idx].helper % 10) + '0';
			printf("%c", c);
		}
		printf("\n");
	}
	printf("\n");
}

void mark_dead_end_distance(struct grid* g, int idx) {
	// idx is walled in by 3 walls
	int dist = 0;
	bool found = true;
	int prev_idx = -1;
	while (found) {
		// check if this cell is dead end, and find next one in process
		int count = 0;
		int next_idx = -1;
		for (int dir = 0; dir < 4; ++dir) {
			int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (g->grid[nidx].type != WALL && nidx != prev_idx) {
				++count;
				next_idx = nidx;
			}
		}
		if (count == 1) {
			g->grid[idx].helper = dist++;
			prev_idx = idx;
			idx = next_idx;
			found = true;
		}
		else
			found = false;
	}
}

bool move_objects(struct grid* g, int idx) {
	bool changed = false;
	// after dist to dead end marked, objects are moved to
	// beginning of hallway
	int stack1[512];
	int sp1 = 0;
	int stack2[512];
	int sp2 = 0;
	int dist = 0;
	// follow path starting from 0, putting idx on both stacks
	while (g->grid[idx].helper == dist) {
		stack1[sp1++] = idx;
		stack2[sp2++] = idx;
		for (int dir = 0; dir < 4; ++dir) {
			int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (g->grid[nidx].type != WALL && g->grid[nidx].helper == dist + 1) {
				idx = nidx;
				break;
			}
		}
		++dist;
	}
	while (sp2) {
		idx = stack2[--sp2];
		if (g->grid[idx].type != EMPTY) {
			int idx_to = stack1[--sp1];
			if (idx != idx_to) {
				g->grid[idx_to].type = g->grid[idx].type;
				g->grid[idx_to].code = g->grid[idx].code;
				if (g->grid[idx].type == KEY)
					g->grid[idx_to].cost = g->grid[idx].cost +
						2 * (g->grid[idx_to].helper - g->grid[idx].helper);
				g->grid[idx].type = EMPTY;
				changed = true;
			}
		}
	}
	return changed;
}


bool close_dead_end(struct grid* g, int idx) {
	bool changed = false;
	int dist = 0;
	// follow path starting from 0
	while (g->grid[idx].helper == dist && g->grid[idx].type != KEY) { // DOOR can be destroyed if it only blocks dead end
		g->grid[idx].type = WALL;
		changed = true;
		for (int dir = 0; dir < 4; ++dir) {
			int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (g->grid[nidx].type != WALL && g->grid[nidx].helper == dist + 1) {
				idx = nidx;
				break;
			}
		}
		++dist;
	}
	return changed;
}

bool find_dead_ends(struct grid* g) {
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].helper = -1;
	int changed = false;
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			if (g->grid[idx].type != WALL) {
				int count = 0;
				for (int dir = 0; dir < 4; ++dir)
					count += g->grid[idx + dir2dy[dir] * g->gridw + dir2dx[dir]].type == WALL ? 1 : 0;
				if (count == 3) {
					mark_dead_end_distance(g, idx);
					changed = move_objects(g, idx) || changed;
					changed = close_dead_end(g, idx) || changed;
				}
			}
		}
	}
	return changed;
}

void get_reachable_stuff(struct grid* g, uint32_t* keys, uint32_t* doors) {
	*keys = 0;
	*doors = 0;

	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].helper = false; // visited

	int idx = g->posy * g->gridw + g->posy;

	struct int_queue* q = create_int_queue();
	int_queue_enqueue(q, idx);
	g->grid[idx].helper = true;

	while (!int_queue_isempty(q)) {
		int idx = int_queue_dequeue(q);
		for (int dir = 0; dir < 4; ++dir) {
			int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (!g->grid[nidx].helper) {
				if (g->grid[nidx].type == EMPTY)
					int_queue_enqueue(q, nidx);
				else if (g->grid[nidx].type == KEY) {
					int_queue_enqueue(q, nidx);
					*keys |= (1 << g->grid[nidx].code);
				}
				else if (g->grid[nidx].type == DOOR)
					*doors |= (1 << g->grid[nidx].code);
				g->grid[nidx].helper = true; // mark as seen
			}
		}
	}
	destroy_int_queue(q);
}
void show_keys_doors(uint32_t keys, uint32_t doors) {
	printf("Keys:  ");
	for (int ii = 0; keys; ++ii) {
		if ((keys & 1) == 1)
			printf("%c", 'a' + ii);
		else
			printf(" ");
		keys >>= 1;
	}
	printf("\nDoors: ");
	for (int ii = 0; doors; ++ii) {
		if ((doors & 1) == 1)
			printf("%c", 'A' + ii);
		else
			printf(" ");
		doors >>= 1;
	}
	printf("\n");
}

/* TODO
void goto_key(struct grid* g, int keycode) {
	int key_idx;
	for (key_idx = 0; key_idx < g->gridw * g->gridh; ++key_idx)
		if (g->grid[key_idx].type == KEY && g->grid[key_idx].code == keycode)
			break;
	if (key_idx == g->gridw * g->gridh) {
		printf("Error: key %c not found\n", keycode + 'a');
		return;
	}
	// create path from key to you
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].helper = INT_MAX;
	struct int_queue* q = create_int_queue();
	int_queue_enqueue(q, key_idx);
	g->grid[key_idx].helper = 0;
	while (!int_queue_isempty(q)) {
		int idx = int_queue_dequeue(q);
		for (int dir = 0; dir < 4; ++dir) {
			int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (g->grid[nidx].type != WALL && g->grid[idx].helper + 1 < g->grid[nidx].helper) {
				g->grid[nidx].helper = g->grid[idx].helper + 1;
				int_queue_enqueue(q, nidx);
			}
		}
	}
	// now walk along path
	int idx = g->posy * g->gridw + g->posx;
	while
}
*/

int main(int argc, char* argv[]) {
	struct grid* grid = create_grid();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		grid_add_line(grid, line);
	free(line);

	show_grid(grid);

	uint32_t keys, doors;
	get_reachable_stuff(grid, &keys, &doors);
	show_keys_doors(keys, doors);

	while (find_dead_ends(grid))
		show_grid(grid);

	get_reachable_stuff(grid, &keys, &doors);
	show_keys_doors(keys, doors);

	destroy_grid(grid);
	return 0;
}
