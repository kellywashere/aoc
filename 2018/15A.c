#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define GRIDSIZE 32

// ordered by reading order!
int dir2dx[] = {0, -1, 1, 0};
int dir2dy[] = {-1, 0, 0, 1};

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


enum cell_type {
	EMPTY,
	WALL,
	ENTITY
};

enum entity_type {
	ELF,
	GOBLIN
};

struct entity {
	enum entity_type type;
	int              attack;
	int              hp;
	int              x;
	int              y;
};

struct cell {
	enum cell_type type;
	struct entity* entity; // NULL if EMPTY or WALL
	// algo helpers
	bool inrange;
	int  dist;
};

struct grid {
	int          gridw;
	int          gridh;
	int          rowcapacity; // max nr of rows reserved in mem
	struct cell* grid;
	int          rounds_played;
};

struct entity* create_entity(enum entity_type type) {
	struct entity* e = malloc(sizeof(struct entity));
	e->type = type;
	e->attack = 3;
	e->hp = 200;
	return e;
}

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	g->rounds_played = 0;
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
		// fprintf(stderr, "Line width inconsistency\n");
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
		g->grid[idx].type = line[col] == '#' ? WALL : line[col] == '.' ? EMPTY : ENTITY;
		g->grid[idx].entity = NULL;
		if (g->grid[idx].type == ENTITY) {
			struct entity* e = create_entity(line[col] == 'E' ? ELF : GOBLIN);
			e->x = col;
			e->y = g->gridh;
			g->grid[idx].entity = e;
		}
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g, bool show_inrange, bool show_dist, int mark_idx) {
	printf("Grid after %d rounds:\n", g->rounds_played);
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			enum cell_type t = g->grid[idx].type;
			struct entity* e = g->grid[idx].entity;
			char c = t == WALL ? '#' : t == EMPTY ? '.' : e->type == ELF ? 'E' : 'G';
			if (show_dist && t == EMPTY) {
				int d = g->grid[idx].dist;
				c = d == INT_MAX ? '*' : d % 10 + '0';
			}
			if (show_inrange && t == EMPTY)
				c = g->grid[idx].inrange ? '?' : c;
			if (idx == mark_idx)
				c = '@';
			printf("%c", c);
		}
		printf("\n");
	}
}

int grid_count_entities(struct grid* g) {
	int count = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		count += g->grid[ii].type == ENTITY ? 1 : 0;
	return count;
}

struct entity** grid_collect_entities(struct grid* g) {
	// finds entities in grid and returns array of entities
	int nr_entities = grid_count_entities(g);
	struct entity** entities = malloc(nr_entities * sizeof(struct entity*));
	int n = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		if (g->grid[ii].type == ENTITY)
			entities[n++] = g->grid[ii].entity;
	return entities;
}

int cmp_entities(const void* a, const void* b) { // qsort fn
	// sort by deadness, then position
	struct entity* ea = *(struct entity**)a;
	struct entity* eb = *(struct entity**)b;
	bool deada = ea->hp <= 0;
	bool deadb = eb->hp <= 0;
	if (deada != deadb)
		return (deada ? 1 : 0) - (deadb ? 1 : 0);
	return ea->y == eb->y ? ea->x - eb->x : ea->y - eb->y;
}

bool has_enemies(struct entity* e, struct entity* entities[], int nr_entities) {
	for (int ii = 0; ii < nr_entities; ++ii)
		if (entities[ii]->type != e->type && entities[ii]->hp > 0)
			return true;
	return false;
}

void mark_inrange(struct grid* g, enum entity_type enemy) {
	int x, y, idx;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].inrange = false;
	for (y = 0; y < g->gridh; ++y) {
		for (x = 0; x < g->gridw; ++x) {
			idx = y * g->gridw + x;
			if (g->grid[idx].type == ENTITY && g->grid[idx].entity->type == enemy) {
				for (int dir = 0; dir < 4; ++dir) {
					int nx = x + dir2dx[dir];
					int ny = y + dir2dy[dir];
					int nidx = ny * g->gridw + nx;
					if (nx >= 0 && nx < g->gridw && ny >= 0 && ny < g->gridh && g->grid[nidx].type != WALL)
						g->grid[nidx].inrange = true;
				}
			}
		}
	}
}

void calc_distances(struct entity* e, struct grid* g) {
	int idx, x, y;
	struct int_queue* q = create_int_queue();

	for (int ii = 0; ii < g->gridw * g->gridh; ++ii)
		g->grid[ii].dist = INT_MAX;

	idx = e->y * g->gridw + e->x;
	g->grid[idx].dist = 0;
	int_queue_enqueue(q, idx);
	while (!int_queue_isempty(q)) {
		idx = int_queue_dequeue(q);
		x = idx % g->gridw;
		y = idx / g->gridw;
		for (int dir = 0; dir < 4; ++dir) {
			int nx = x + dir2dx[dir];
			int ny = y + dir2dy[dir];
			int nidx = ny * g->gridw + nx;
			if (nx >= 0 && nx < g->gridw && ny >= 0 && ny < g->gridh && g->grid[nidx].type == EMPTY) {
				if (g->grid[idx].dist + 1 < g->grid[nidx].dist) {
					g->grid[nidx].dist = g->grid[idx].dist + 1;
					int_queue_enqueue(q, nidx);
				}
			}
		}
	}

	destroy_int_queue(q);
}

int backtrack_for_pathdir(struct grid* g, int idx, int e_idx) {
	int x, y;
	int last_idx = -1;
	while (idx != e_idx) {
		last_idx = idx;
		x = idx % g->gridw;
		y = idx / g->gridw;
		int dist = g->grid[idx].dist;
		for (int dir = 0; idx == last_idx && dir < 4; ++dir) {
			int nx = x + dir2dx[dir];
			int ny = y + dir2dy[dir];
			int nidx = ny * g->gridw + nx;
			if (nx >= 0 && nx < g->gridw && ny >= 0 && ny < g->gridh && g->grid[nidx].dist == dist - 1)
				idx = nidx;
		}
	}
	return last_idx;
}

void entity_move(struct entity* e, struct grid* g) {
	int e_idx = e->y * g->gridw + e->x;
	// step 1: mark adjacent cells of enemies
	enum entity_type enemy = e->type == ELF ? GOBLIN : ELF;
	mark_inrange(g, enemy);
	if (g->grid[e_idx].inrange)
		return; // don't move when already in range
	// step 2: find reachable cells and their distance
	calc_distances(e, g);
	// step 3: find closest target
	int mindist = INT_MAX;
	int idx_mindist = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii) { // reading order
		if (g->grid[ii].inrange && g->grid[ii].dist < mindist) {
			mindist = g->grid[ii].dist;
			idx_mindist = ii;
		}
	}
	if (mindist != INT_MAX) {
		// step 4: backtrack to e for pathfinding
		int step_idx = backtrack_for_pathdir(g, idx_mindist, e_idx);
		// step 5: move entity to step_idx
		g->grid[e_idx].type = EMPTY;
		g->grid[e_idx].entity = NULL;
		g->grid[step_idx].type = ENTITY;
		g->grid[step_idx].entity = e;
		e->x = step_idx % g->gridw;
		e->y = step_idx / g->gridw;
	}
}

void entity_attack(struct entity* e, struct grid* g) {
	// step 1: find target
	int min_hp = INT_MAX;
	int enemy_idx = 0;
	struct entity* enemy = NULL;
	for (int dir = 0; dir < 4; ++dir) {
		int nx = e->x + dir2dx[dir];
		int ny = e->y + dir2dy[dir];
		int nidx = ny * g->gridw + nx;
		if (nx >= 0 && nx < g->gridw && ny >= 0 && ny < g->gridh && g->grid[nidx].type == ENTITY &&
		             g->grid[nidx].entity->type != e->type && g->grid[nidx].entity->hp < min_hp) {
			enemy = g->grid[nidx].entity;
			min_hp = enemy->hp;
			enemy_idx = nidx;
		}
	}
	// step 2: attack target
	if (enemy) {
		enemy->hp -= e->attack;
		if (enemy->hp <= 0)
			g->grid[enemy_idx].type = EMPTY;
	}
}

void run_game(struct grid* grid, struct entity* entities[], int nr_entities) {
	// show_grid(grid, false, false, -1);

	bool game_over = false;
	while (!game_over) {
		// round
		// sort entities for move order
		qsort(entities, nr_entities, sizeof(struct entity*), cmp_entities);
		// loop over entities
		for (int ii = 0; ii < nr_entities; ++ii) {
			if (entities[ii]->hp > 0) {
				if (!has_enemies(entities[ii], entities, nr_entities)) {
					game_over = true;
					break;
				}
				entity_move(entities[ii], grid);
				entity_attack(entities[ii], grid);
			}
		}
		if (!game_over)
			++grid->rounds_played;
		// show_grid(grid, false, false, -1);
		// getchar();
	}
}

int main(int argc, char* argv[]) {
	// grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(bool));
	struct grid* grid = create_grid();
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!grid_add_line(grid, line))
			break;
	}
	free(line);

	// collect entities in array
	int nr_entities = grid_count_entities(grid);
	struct entity** entities = grid_collect_entities(grid);
	run_game(grid, entities, nr_entities);

	int hp_remaining = 0;
	for (int ii = 0; ii < grid->gridw * grid->gridh; ++ii)
		hp_remaining += grid->grid[ii].type == ENTITY ? grid->grid[ii].entity->hp : 0;
	int prod = grid->rounds_played * hp_remaining;
	// printf("%d * %d = %d\n", grid->rounds_played, hp_remaining, prod);
	printf("%d\n", prod);

	destroy_grid(grid);
	for (int ii = 0; ii < nr_entities; ++ii)
		free(entities[ii]);
	free(entities);
	return 0;
}
