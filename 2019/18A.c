#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>

// term colors
#define COL_RESET "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"

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

struct player {
	int      posx;
	int      posy;
	uint32_t keys; // bitfield
	int      totcost;
};

int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, -1, 0, 1};

struct grid {
	int           gridw;
	int           gridh;
	int           rowcapacity; // max nr of rows reserved in mem
	struct cell*  grid;
	struct player you;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	g->you.posx = -1;
	g->you.posy = -1;
	g->you.keys = 0;
	g->you.totcost = 0;
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
			g->you.posx = col;
			g->you.posy = g->gridh;
		}
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g, bool show_helper) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			if (x == g->you.posx && y == g->you.posy)
				printf(YEL "@" COL_RESET);
			else {
				int idx = y * g->gridw + x;

				switch (g->grid[idx].type) {
					case WALL:
						putchar('#');
						break;
					case EMPTY:
						if (show_helper && g->grid[idx].helper >= 0)
							putchar(g->grid[idx].helper % 10 + '0');
						else
							putchar('.');
						break;
					case KEY:
						printf(GRN "%c" COL_RESET, g->grid[idx].code + 'a');
						break;
					case DOOR:
						printf(RED "%c" COL_RESET, g->grid[idx].code + 'A');
						break;
				}
			}
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

bool find_key(struct grid* g, int key_code, int* idx) {
	int key_idx;
	for (key_idx = 0; key_idx < g->gridw * g->gridh; ++key_idx)
		if (g->grid[key_idx].type == KEY && g->grid[key_idx].code == key_code)
			break;
	if (key_idx < g->gridw * g->gridh) {
		*idx = key_idx;
		return true;
	}
	return false;
}

void mark_distances_to_cell(struct grid* g, int from_idx) {
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].helper = INT_MAX; // distances

	struct int_queue* q = create_int_queue();

	int_queue_enqueue(q, from_idx);
	g->grid[from_idx].helper = 0;

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
	destroy_int_queue(q);
}

int find_next_on_path(struct grid* g, int idx) {
	// finds neighbor of idx with helper that is one lower
	int h = g->grid[idx].helper - 1;
	for (int dir = 0; dir < 4; ++dir) {
		int nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
		if (g->grid[nidx].helper == h)
			return nidx;
	}
	return -1;
}

void remove_useless_doors(struct grid* g) {
	// removes doors that do not block a key
	uint32_t useless_doors = 0x3ffffff; // 26 ones
	for (int key_code = 0; key_code < 26; ++key_code) {
		int key_idx = 0;
		if (find_key(g, key_code, &key_idx)) {
			mark_distances_to_cell(g, key_idx);
			// walk dist path in reverse, check encountered doors/keys
			int idx = g->you.posy * g->gridw + g->you.posx;
			while (g->grid[idx].helper) {
				idx = find_next_on_path(g, idx);
				if (g->grid[idx].type == DOOR) // this door really blocks a key
					useless_doors &= ~(1 << g->grid[idx].code);
			}
		}
	}
	for (int door_code = 0; door_code < 26; ++door_code) {
		if ( (useless_doors & (1 << door_code)) ) {
			for (int ii = 0; ii < g->gridw * g->gridh; ++ii) {
				if (g->grid[ii].type == DOOR && g->grid[ii].code == door_code) {
					printf("Removing useless door %c\n", door_code + 'A');
					g->grid[ii].type = EMPTY;
				}
			}
		}
	}
}

bool is_dead_end(struct grid* g, int idx) {
	int count = 0;
	if (g->grid[idx].type == EMPTY) {
		for (int dir = 0; dir < 4; ++dir)
			count += g->grid[idx + dir2dy[dir] * g->gridw + dir2dx[dir]].type == WALL ? 1 : 0;
	}
	return count == 3;
}

void remove_redundant_keys(struct grid* g) {
	// remove keys that have no corresponding door, unless they are the last key in hallway
	int you_idx = g->you.posy * g->gridw + g->you.posx;
	mark_distances_to_cell(g, you_idx);
	for (int key_code = 0; key_code < 26; ++key_code) {
		int idx = 0;
		if (find_key(g, key_code, &idx)) {
			// walk path from key to you
			while (g->grid[idx].helper && g->grid[idx].type != DOOR) {
				idx = find_next_on_path(g, idx);
				if (g->grid[idx].type == KEY) { // this key is between key key_code and you
					// change door codes before removing key
					for (int ii = 0; ii < g->gridw * g->gridh; ++ii) {
						if (g->grid[ii].type == DOOR && g->grid[ii].code == g->grid[idx].code) {
							printf("Changing door %c to %c\n", 'A' + g->grid[ii].code, 'A' + key_code);
							g->grid[ii].code = key_code;
						}
					}
					printf("Removing redundant key %c\n", 'a' + g->grid[idx].code);
					g->grid[idx].type = EMPTY;
				}
			}
		}
	}
}


void remove_dead_ends(struct grid* g) {
	bool changed = true;
	int you_idx = g->you.posy * g->gridw + g->you.posx;
	while (changed) {
		changed = false;
		for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
			g->grid[ii].helper = -1;
		for (int y = 0; y < g->gridh; ++y) {
			for (int x = 0; x < g->gridw; ++x) {
				int idx = y * g->gridw + x;
				while (idx != you_idx && is_dead_end(g, idx)) {
					changed = true;
					g->grid[idx].type = WALL;
					int nidx = -1;
					for (int dir = 0; dir < 4; ++dir) {
						nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
						if (g->grid[nidx].type != WALL)
							break;
					}
					idx = nidx;
				}
			}
		}
	}
}

void get_reachable_stuff(struct grid* g, uint32_t* keys, uint32_t* doors) {
	*keys = 0;
	*doors = 0;

	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].helper = false; // visited

	int idx = g->you.posy * g->gridw + g->you.posx;
	//printf("@ (%d,%d)\n", idx % g->gridw, idx / g->gridw);

	struct int_queue* q = create_int_queue();
	int_queue_enqueue(q, idx);
	g->grid[idx].helper = true;

	while (!int_queue_isempty(q)) {
		idx = int_queue_dequeue(q);
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

void grid_open_door(struct grid* g, int code) {
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		if (g->grid[ii].type == DOOR && g->grid[ii].code == code)
			g->grid[ii].type = EMPTY;
}

void goto_key(struct grid* g, int keycode) {
	int key_idx;
	for (key_idx = 0; key_idx < g->gridw * g->gridh; ++key_idx)
		if (g->grid[key_idx].type == KEY && g->grid[key_idx].code == keycode)
			break;
	if (key_idx == g->gridw * g->gridh) {
		printf("Error: key %c not found\n", keycode + 'a');
		return;
	}
	mark_distances_to_cell(g, key_idx);

	// walk dist path in reverse, check encountered doors/keys
	int idx = g->you.posy * g->gridw + g->you.posx;
	int dist = g->grid[idx].helper;
	while (dist) {
		int nidx = -1;
		for (int dir = 0; dir < 4; ++dir) {
			nidx = idx + dir2dy[dir] * g->gridw + dir2dx[dir];
			if (g->grid[nidx].helper == dist - 1) {
				--dist;
				break;
			}
		}
		if (nidx == -1) {
			printf("Something went horribly wrong!\n");
			return;
		}
		if (g->grid[nidx].type == DOOR) {
			printf("Stopped in my tracks by door %c\n", 'A' + g->grid[nidx].code);
			break;
		}
		idx = nidx;
		if (g->grid[idx].type == KEY) {
			printf("Picking up key %c\n", 'a' + g->grid[idx].code);
			g->you.keys |= (1 << g->grid[idx].code);
			g->grid[idx].type = EMPTY;
			grid_open_door(g, g->grid[idx].code);
		}
	}
	g->you.posx = idx % g->gridw;
	g->you.posy = idx / g->gridw;
}

int main(int argc, char* argv[]) {
	struct grid* g = create_grid();

	FILE* fp = fopen("./input18.txt", "r");
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) != -1)
		grid_add_line(g, line);
	free(line);
	fclose(fp);


	char c = '\0';
	while (c != '@') {
		/*
		while (find_dead_ends(g))
			;
		*/

		remove_useless_doors(g);
		remove_dead_ends(g);
		// remove_redundant_keys(g);

		show_grid(g, false);

		uint32_t keys, doors;
		get_reachable_stuff(g, &keys, &doors);
		printf("I can reach:\n");
		show_keys_doors(keys, doors);
		printf("\n");

		for (int key_code = 0; key_code < 26; ++key_code) {
			int key_idx = 0;
			if (find_key(g, key_code, &key_idx)) {
				printf("Key %c: ", 'a' + key_code);
				mark_distances_to_cell(g, key_idx);
				// walk dist path in reverse, check encountered doors/keys
				int idx = g->you.posy * g->gridw + g->you.posx;
				while (g->grid[idx].helper) {
					idx = find_next_on_path(g, idx);
					if (g->grid[idx].type == DOOR)
						printf("%c, ", 'A' + g->grid[idx].code);
					/*
					else if (g->grid[idx].type == KEY)
						printf("%c, ", 'a' + g->grid[idx].code);
					*/
				}
				printf("\n");
			}
		}

		printf("Which key shall I grab? :  ");
		c = getchar();
		if (c >= 'a' && c <= 'z')
			goto_key(g, c - 'a');
	}


	/*

	while (find_dead_ends(grid))
		show_grid(grid, true);

	get_reachable_stuff(grid, &keys, &doors);
	show_keys_doors(keys, doors);
	*/

	destroy_grid(g);
	return 0;
}
