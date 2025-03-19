#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
/*
How to encode state in an id nr that can be reversed into a board state?
And: how to create hash?

# encoding each cell state into id

Problem A:
there are 7 hall cells and 8 home cells
Each have 5 possiblities (empty, 4 pod types)
This leads to 5^15 --> 35 bits

Problem B:
there are 7 hall cells and 16 home cells
This leads to 5^23 --> 54 bits

So entire state can be stored in uint64_t, and is reversible

*/

#define ABS(x) ((x)<0?(-(x)):(x))

enum cellstate {
	EMPTY = 0,
	A     = 1,
	B     = 2,
	C     = 3,
	D     = 4,
	NR_CELL_STATES
};
#define NR_POD_TYPES  (NR_CELL_STATES - 1)

// some helper arrays for easier cost calc
int cost_per_step[] = {0, 1, 10, 100, 1000};
int hallx[] = {0, 1, 3, 5, 7, 9, 10};
int homex[] = {2, 4, 6, 8};


int g_debugcount = 0; // for counting entries in hash table


#define NR_HALL_CELLS     7
#define NR_HOME_ROWS      4

#define NR_HASH_BITS     20
#define HASHTABLE_SZ     (1<<NR_HASH_BITS)

#define MAX_NEIGHBORS    32

// struct for use in hash table & in neighbor gen
struct id_val {
	uint64_t id;   // full (reversable) state id
	int      val;  // e.g. cost
};

uint64_t hash(uint64_t key) {
	// from murmur mixer
	key ^= (key >> 33);
	key *= 0xff51afd7ed558ccd;
	key ^= (key >> 33);
	key *= 0xc4ceb9fe1a85ec53;
	key ^= (key >> 33);
	return key;
}

struct state {
	enum cellstate hall[NR_HALL_CELLS];
	enum cellstate home[NR_HOME_ROWS * NR_POD_TYPES]; // idx: row + type * NR_HOME_ROWS
};

uint64_t state_to_id(struct state* s) {
	uint64_t id = 0;
	for (int ii = 0; ii < NR_HALL_CELLS; ++ii)
		id = NR_CELL_STATES * id + s->hall[ii];
	for (int ii = 0; ii < NR_HOME_ROWS * NR_POD_TYPES; ++ii)
		id = NR_CELL_STATES * id + s->home[ii];
	return id;
}

uint64_t solved_id(void) {
	uint64_t id = 0;
	for (int homecol = 0; homecol < NR_POD_TYPES; ++homecol)
		for (int row = 0; row < NR_HOME_ROWS; ++row)
			id = NR_CELL_STATES * id + (homecol + A);
	return id;
}

void id_to_state(uint64_t id, struct state* s) {
	for (int ii = NR_HOME_ROWS * NR_POD_TYPES - 1; ii >= 0; --ii) {
		s->home[ii] = id % NR_CELL_STATES;
		id /= NR_CELL_STATES;
	}
	for (int ii = NR_HALL_CELLS - 1; ii >= 0; --ii) {
		s->hall[ii] = id % NR_CELL_STATES;
		id /= NR_CELL_STATES;
	}
}

/************************* MINHEAP *******************************/
// NOTE: this implementation allows searching for value, and
// changing key of a value to allow A-star search !
struct minheap_el {
	int      key;
	uint64_t val; // val is id# here, so 64 bit needed
};

struct minheap {
	struct minheap_el* mem;      // array-based min heap
	unsigned int       size;     // nr elements currently in heap
	unsigned int       capacity; // capacity in nr of elements
};

struct minheap* create_minheap() {
	struct minheap* heap = malloc(sizeof(struct minheap));
	heap->capacity = 127; // 7 layers start size
	heap->mem = malloc(sizeof(struct minheap_el) * heap->capacity);
	heap->size = 0;
	return heap;
}

void destroy_minheap(struct minheap* heap) {
	if (heap)
		free(heap->mem);
	free(heap);
}

static void minheap_swap(struct minheap* heap, int idx1, int idx2) {
	struct minheap_el t = heap->mem[idx1];
	heap->mem[idx1] = heap->mem[idx2];
	heap->mem[idx2] = t;
}

static void minheap_bubble_up(struct minheap* heap, int idx) {
	int idx_parent = (idx - 1) / 2;
	while (idx != 0 && heap->mem[idx].key < heap->mem[idx_parent].key) {
		minheap_swap(heap, idx, idx_parent);
		idx = idx_parent;
		idx_parent = (idx - 1) / 2;
	}
}

static void minheap_bubble_down(struct minheap* heap, int idx) {
	int key_this = heap->mem[idx].key;
	bool done = false;
	while (!done) {
		done = true;
		int smallest_key = key_this;
		int idx_smallest = idx;
		// find smallest between idx, left and right child
		int idx_child;
		for (idx_child = (idx * 2) + 1; idx_child <= (idx * 2) + 2; ++idx_child) {
			if (idx_child < heap->size) {
				int k = heap->mem[idx_child].key;
				if (k < smallest_key) {
					smallest_key = k;
					idx_smallest = idx_child;
				}
			}
		}
		if (idx_smallest != idx) {
			done = false;
			minheap_swap(heap, idx, idx_smallest);
			idx = idx_smallest;
		}
	}
}

void minheap_insert(struct minheap* heap, int key, uint64_t val) {
	// check capacity
	if (heap->size == heap->capacity) {
		heap->capacity = 2 * heap->capacity + 1; // 1 more layer
		heap->mem = realloc(heap->mem, sizeof(struct minheap_el) * heap->capacity);
	}
	int idx = heap->size; // insertion idx
	// copy info into new last element
	heap->mem[idx].key = key;
	heap->mem[idx].val = val;
	++heap->size;
	minheap_bubble_up(heap, idx);
}

int minheap_extract(struct minheap* heap, uint64_t* val) { // returns key, copies val to *val
	if (heap->size == 0) {
		fprintf(stderr, "Error: extracting from empty heap\n");
		exit(1);
	}
	int retKey = heap->mem[0].key;
	if (val)
		*val = heap->mem[0].val;

	--heap->size;
	if (heap->size > 0) {
		// copy ex-last element to root position
		heap->mem[0] = heap->mem[heap->size];
		minheap_bubble_down(heap, 0);
	}
	return retKey;
}

bool minheap_contains_val(struct minheap* heap, uint64_t val) {
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val)
			return true;
	}
	return false;
}

bool minheap_update_key_of_val(struct minheap* heap, int key, uint64_t val) {
	// Note: only changes key of first val found that had a different key
	// returns whether change was made
	bool changed = false;
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val && heap->mem[idx].key != key) {
			int oldkey = heap->mem[idx].key;
			heap->mem[idx].key = key;
			changed = true;
			if (key < oldkey)
				minheap_bubble_up(heap, idx);
			else
				minheap_bubble_down(heap, idx);
		}
	}
	return changed;
}

/************************* END MINHEAP ***************************/


void read_state_from_stdin(struct state* s) {
	char *line = NULL;
	size_t len = 0;
	// skip 1st & 2nd line
	if (getline(&line, &len, stdin) == -1) return;
	if (getline(&line, &len, stdin) == -1) return;
	// read initial positions from next NR_HOME_ROWS lines
	for (int row = 0; row < NR_HOME_ROWS; ++row) {
		if (row == 1)
			strcpy(line, "  #D#C#B#A#");
		else if (row == 2)
			strcpy(line, "  #D#B#A#C#");
		else if (getline(&line, &len, stdin) == -1) return;
		for (int type = 0; type < NR_POD_TYPES; ++type) {
			s->home[row + type * NR_HOME_ROWS] = line[3 + 2 * type] - 'A' + 1;
		}
	}
	free(line);
}

void print_state(struct state* s) {
	printf("#############\n#");
	for (int ii = 0; ii < NR_HALL_CELLS; ++ii) {
		printf("%c", s->hall[ii] == EMPTY ? '.' : s->hall[ii] + 'A' - 1);
		if (ii >= 1 && ii < 5)
			printf(" ");
	}
	printf("#\n###");
	for (int row = 0; row < NR_HOME_ROWS; ++row) {
		for (int type = 0; type < NR_POD_TYPES; ++type) {
			int idx = row + type * NR_HOME_ROWS;
			printf("%c#", s->home[idx] == EMPTY ? '.' : s->home[idx] + 'A' - 1);
		}
		if (row == 0)
			printf("##");
		printf("\n  #");
	}
	for (int ii = 0; ii < NR_POD_TYPES; ++ii)
		printf("##");
	printf("\n");
}

void gen_neighbors(uint64_t id, struct id_val* neighbors, size_t *neighbors_sz) {
	struct state s;
	*neighbors_sz = 0;
	id_to_state(id, &s);

/*
#############
#01.2.3.4.56#
###0#1#2#3###
  #0#1#2#3#
  #########
*/

	// check each home.
	// If starting at bottom all pods are home, but there is empty spot at top, we can
	// see if the right pod can travel from hall to empty.
	// If not all pods in home space are of correct type, we can move the top one into hallway
	for (int homecol = 0; homecol < NR_POD_TYPES; ++homecol) {
		enum cellstate podtype = A + homecol;
		bool only_right_type = true;
		int first_occupied_row = NR_HOME_ROWS;

		for (int row = NR_HOME_ROWS - 1; row >= 0; --row) {
			int idx = row + homecol * NR_HOME_ROWS;
			if (s.home[idx] == EMPTY)
				break;
			first_occupied_row = row;
			if (s.home[idx] != podtype)
				only_right_type = false;
		}

		if (only_right_type && first_occupied_row > 0) { // move hall --> home?
			// see if we can "see" correct pod in hall from this home col
			// scan left
			for (int c = homecol + 1; c >= 0; --c) {
				if (s.hall[c] == EMPTY)
					continue;
				if (s.hall[c] == podtype) { // found one
					s.hall[c] = EMPTY;
					int idx = first_occupied_row - 1 + homecol * NR_HOME_ROWS;
					s.home[idx] = podtype;
					neighbors[*neighbors_sz].id = state_to_id(&s);
					int steps = first_occupied_row + (homex[homecol] - hallx[c]);
					neighbors[*neighbors_sz].val = steps * cost_per_step[podtype];
					++(*neighbors_sz);
					s.home[idx] = EMPTY;
					s.hall[c] = podtype;
				}
				break; // we break out of for loop for any pod type
			}
			// scan right
			for (int c = homecol + 2; c < NR_HALL_CELLS; ++c) {
				if (s.hall[c] == EMPTY)
					continue;
				if (s.hall[c] == podtype) { // found one
					s.hall[c] = EMPTY;
					int idx = first_occupied_row - 1 + homecol * NR_HOME_ROWS;
					s.home[idx] = podtype;
					neighbors[*neighbors_sz].id = state_to_id(&s);
					int steps = first_occupied_row + (hallx[c] - homex[homecol]);
					neighbors[*neighbors_sz].val = steps * cost_per_step[podtype];
					++(*neighbors_sz);
					s.home[idx] = EMPTY;
					s.hall[c] = podtype;
				}
				break; // we break out of for loop for any pod type
			}
		}
		if (!only_right_type) { // move top into hall
			int idx = first_occupied_row + homecol * NR_HOME_ROWS;
			enum cellstate movepod = s.home[idx];
			s.home[idx] = EMPTY;
			// all moves going left into hall
			for (int c = homecol + 1; c >= 0; --c) {
				if (s.hall[c] != EMPTY)
					break;
				s.hall[c] = movepod;
				neighbors[*neighbors_sz].id = state_to_id(&s);
				int steps = (first_occupied_row + 1) + (homex[homecol] - hallx[c]);
				neighbors[*neighbors_sz].val = steps * cost_per_step[movepod];
				++(*neighbors_sz);
				s.hall[c] = EMPTY;
			}
			// all moves going right into hall
			for (int c = homecol + 2; c < NR_HALL_CELLS; ++c) {
				if (s.hall[c] != EMPTY)
					break;
				s.hall[c] = movepod;
				neighbors[*neighbors_sz].id = state_to_id(&s);
				int steps = (first_occupied_row + 1) + (hallx[c] - homex[homecol]);
				neighbors[*neighbors_sz].val = steps * cost_per_step[movepod];
				++(*neighbors_sz);
				s.hall[c] = EMPTY;
			}
			s.home[first_occupied_row + homecol * NR_HOME_ROWS] = movepod;
		}
	}
}

// Heuristic for A-star
int hscore(struct state* s) { 
/*
#############
#01.2.3.4.56#
###0#1#2#3###
  #0#1#2#3#
  #########
*/
	int h = 0;
	for (int homecol = 0; homecol < NR_POD_TYPES; ++homecol) {
		enum cellstate podtype = A + homecol;
		bool allhome = true;

		for (int row = NR_HOME_ROWS - 1; row >= 0; --row) {
			int idx = row + homecol * NR_HOME_ROWS;
			if (s->home[idx] == EMPTY)
				break;
			if (s->home[idx] != podtype) {
				allhome = false;
				int homedist = ((s->home[idx] - A) - homecol) * 2;
				int steps = row + 2 + ABS(homedist);
				h += steps * cost_per_step[s->home[idx]];
			}
			else if (!allhome) { // this guy has to move out to make room
				h += (2 * (row + 1) + 1) * cost_per_step[s->home[idx]];
			}
		}
	}
	for (int c = 0; c < NR_HALL_CELLS; ++c) {
		if (s->hall[c] != EMPTY) {
			int homecol = s->hall[c] - A;
			int homedist = hallx[c] - homex[homecol];
			h += (ABS(homedist) + 1) * cost_per_step[s->hall[c]];
		}
	}
	return h;
}

/*
bool hashtable_add(struct id_val* ht, uint64_t id, int val) {
	int idx = hash(id) & ((1 << NR_HASH_BITS) - 1);
	if (ht[idx].id == 0 || ht[idx].id == id) {
		ht[idx].id = id;
		ht[idx].val = val;
		++g_debugcount;
		return true;
	}
	fprintf(stderr, "Hash collision id = %" PRIu64 " at idx %d, already has id %" PRIu64 "\n", id, idx, ht[idx].id);
	fprintf(stderr, "hash(%" PRIu64 ") = 0x%" PRIx64 "; hash(%" PRIu64 ") = 0x%" PRIx64 "\n",
			id, hash(id), ht[idx].id, hash(ht[idx].id));
	fprintf(stderr, "Hashtable has %d entries out of %d\n", g_debugcount, HASHTABLE_SZ);

	return false;
}

bool hashtable_get(struct id_val* ht, uint64_t id, int* val) {
	int idx = hash(id) & ((1 << NR_HASH_BITS) - 1);
	if (ht[idx].id != id) {
		//fprintf(stderr, "Element with id %" PRIu64 " not in hashtable\n", id);
		return false;
	}
	*val = ht[idx].val;
	return true;
}
*/

bool hashtable_add(struct id_val* ht, uint64_t id, int val) {
	int idx = hash(id) & ((1 << NR_HASH_BITS) - 1);
	int idx0 = idx;
	while (ht[idx].id != 0 && ht[idx].id != id) {
		++idx;
		if (idx == idx0) {
			fprintf(stderr, "Hashtable full\n");
			return false; // table full
		}
	}
	ht[idx].id = id;
	ht[idx].val = val;
	++g_debugcount;
	return true;
}

bool hashtable_get(struct id_val* ht, uint64_t id, int* val) {
	int idx = hash(id) & ((1 << NR_HASH_BITS) - 1);
	int idx0 = idx;
	while (ht[idx].id != 0 && ht[idx].id != id) {
		++idx;
		if (idx == idx0)
			break;
	}
	if (ht[idx].id == 0)
		return false; // not in table
	if (ht[idx].id == id) {
		*val = ht[idx].val;
		return true;
	}
	fprintf(stderr, "Hashtable full and does not have entry we are looking for\n");
	return false; // table full
}

int find_target(uint64_t id, uint64_t target_id) {
	// A-star search algo
	// we use hash fn to allow indexing gscore[]

	struct state s;

	struct id_val neighbors[MAX_NEIGHBORS];
	size_t neighbors_sz = 0;

	struct minheap* open_set = create_minheap();
	struct id_val* gscore = malloc(HASHTABLE_SZ * sizeof(*gscore));
	int fscore;
	for (int ii = 0; ii < HASHTABLE_SZ; ++ii) {
		gscore[ii].id = 0;
		gscore[ii].val = INT_MAX;
	}

	hashtable_add(gscore, id, 0);
	id_to_state(id, &s);
	fscore = hscore(&s);

	minheap_insert(open_set, fscore, id);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &id);
		int g;
		hashtable_get(gscore, id, &g);

		if (id == target_id) {
			cost = g;
		}
		else {
			gen_neighbors(id, neighbors, &neighbors_sz);

			// for each neighbor of current
			for (int ii = 0; ii < neighbors_sz; ++ii) {
				int gnext;
				if (!hashtable_get(gscore, neighbors[ii].id, &gnext))
					gnext = INT_MAX;

				int tentative_gscore = g + neighbors[ii].val;
				if (tentative_gscore < gnext) {
					hashtable_add(gscore, neighbors[ii].id, tentative_gscore);

					struct state sn;
					id_to_state(neighbors[ii].id, &sn);
					fscore = tentative_gscore + hscore(&sn);
					if (!minheap_contains_val(open_set, neighbors[ii].id))
						minheap_insert(open_set, fscore, neighbors[ii].id);
					else
						minheap_update_key_of_val(open_set, fscore, neighbors[ii].id); // Nasty...
				}
			}
		}
	}

	destroy_minheap(open_set);
	return cost;
}

int main(int argc, char* argv[]) {
	struct state s = {0};
	read_state_from_stdin(&s);
	uint64_t id = state_to_id(&s);

	/*
	uint64_t id = 184096;
	id_to_state(id, &s);
	print_state(&s);
	*/

	uint64_t idsolved = solved_id();

	/*
	struct id_val neighbors[MAX_NEIGHBORS];
	size_t neighbors_sz = 0;

	uint64_t cost = 0;
	while (state_to_id(&s) != idsolved) {
		printf("Current state:\n");
		print_state(&s);
		printf("Heuristic cost: %d\n", hscore(&s));

		gen_neighbors(state_to_id(&s), neighbors, &neighbors_sz);
		for (int ii = 0; ii < neighbors_sz; ++ii) {
			struct state snext;
			id_to_state(neighbors[ii].id, &snext);
			printf("\n#%d\n", ii);
			print_state(&snext);
			printf("Cost: %d\n", neighbors[ii].val);
		}

		int x = neighbors_sz;
		while (x >= neighbors_sz)
			scanf("%d", &x);

		id_to_state(neighbors[x].id, &s);
		cost += neighbors[x].val;
	}

	printf("Total cost: %" PRIu64 "\n", cost);
	*/
	int cost = find_target(id, idsolved);
	printf("%d\n", cost);

	return 0;
}

