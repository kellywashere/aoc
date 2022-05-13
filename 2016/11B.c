#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// encoding state in a number s:
// groups of 4 bits indicating gen (2 MSB of group) and
// chip floor (2 LSB of group).
// then 2 MSBs of s are elevator location
// input file shows 5 elements, so we have 5 * 4 + 2 bits
// 22 bit nr for state envoding (4 million states)
// of course here floors are labeled 0..3 and not 1..4
// target state: all bits set
// invalid state: 1 or more unpowered chips on a floor with incompatible gen

#define MAX_ELEMENTS 6
#define NO_STATE (-1)

/* ************ QUEUE ************** */
struct state_queue {
	uint32_t  capacity; // max nr elements in q mem region
	uint32_t  front;
	uint32_t  back;
	uint32_t* q; // memory for queue data
};

struct state_queue* new_state_queue() {
	struct state_queue* q = malloc(sizeof(struct state_queue));
	q->capacity = 1024;
	q->q = malloc(q->capacity * sizeof(uint32_t));
	q->front = 0;
	q->back = 0;
	return q;
}

void destroy_state_queue(struct state_queue* q) {
	if (q) {
		free(q->q);
		free(q);
	}
}

uint32_t state_queue_size(struct state_queue* q) {
	return (q->back + q->capacity - q->front) % q->capacity;
}

bool state_queue_isempty(struct state_queue* q) {
	return q->back == q->front;
}

uint32_t state_queue_dequeue(struct state_queue* q) {
	uint32_t e = q->q[q->front];
	q->front = (q->front + 1) % q->capacity;
	return e;
}

void state_queue_enqueue(struct state_queue* q, uint32_t e) {
	if (state_queue_size(q) + 1 >= q->capacity) { // enlarge capcity
		q->q = realloc(q->q, 2 * q->capacity * sizeof(uint32_t));
		if (q->back < q->front) {
			if (q->back > 0)
				memcpy(q->q + q->capacity, q->q, q->back * sizeof(uint32_t));
			q->back += q->capacity;
		}
		q->capacity *= 2;
	}
	q->q[q->back] = e;
	q->back = (q->back + 1) % q->capacity;
}
/* ************ END QUEUE ************** */

int element_to_idx(char* elname, char* elements[]) {
	// concert name of element into an index nr
	// if a not in elements array, adds it
	int ii;
	for (ii = 0; elements[ii] != NULL; ++ii)
		if (!strcmp(elements[ii], elname))
			return ii;
	int l = strlen(elname);
	elements[ii] = malloc((l + 1) * sizeof(char));
	strcpy(elements[ii], elname);
	elements[ii + 1] = NULL;
	return ii;
}

uint32_t nr_states_from_elements(int nr_elements) {
	// nr possible states = 4 * 16^nr_elements
	uint32_t nr_states = 4; // 4 elevator positions
	for (int ii = 0; ii < nr_elements; ++ii)
		nr_states *= 4 * 4; // 4 floors for gen and 4 floors for chip
	return nr_states;
}

void show_state(uint32_t s, int nr_el) {
	int elevator = s >> (4 * nr_el);
	for (int floor = 3; floor >= 0; --floor) {
		if (elevator == floor)
			printf(" <> ");
		else
			printf(" .. ");
		for (int e = nr_el - 1; e >= 0; --e) {
			int gen_chip = (s >> (4 * e)) & 0x0f;
			int chipfloor = gen_chip & 0x03;
			int genfloor = gen_chip >> 2;
			printf(" %c%c", (genfloor == floor ? ('A' + e) : '.'),
					(chipfloor == floor ? ('a' + e) : '.'));
		}
		printf("\n");
	}
}

bool is_safe_state(uint32_t s, int nr_el) {
	// state UNsafe when both:
	// 1. a chip is on a different floor than its generator
	// 2. there is another generator on the same floor as that chip
	for (int e = 0; e < nr_el; ++e) {
		int gen_chip = (s >> (4 * e)) & 0x0f;
		int chipfloor = gen_chip & 0x03;
		int genfloor = gen_chip >> 2;
		if (chipfloor != genfloor) { // unpowered chip
			// check if different gen on chipfloor
			for (int g = 0; g < nr_el; ++g) {
				if (g == e)
					continue;
				genfloor = (s >> (4 * g + 2)) & 0x03;
				if (genfloor == chipfloor)
					return false; // chip is fried !
			}
		}
	}
	return true; // safe
}

static const char* countstrs[] = {"first", "second", "third", "fourth"};
static const char* genstr = "generator";
static const char* chipstr = "-compatible";

void process_line(char* line, char* elements[], uint32_t* s) {
	printf("%s", line);

	int elidx;
	int floornr;
	for (floornr = 0; floornr < 4; ++floornr)
		if (strstr(line, countstrs[floornr]))
			break;
	if (floornr > 3) {
		fprintf(stderr, "Could not determine floornr from line!\n");
		return;
	}
	if (strstr(line, "nothing"))
		return;
	// process generators
	char* loc = strstr(line, genstr);
	while (loc) {
		// move back until beginning of element name
		char* elname = loc - 1;
		while (*(elname - 1) != ' ')
			--elname;
		*(loc - 1) = '\0'; // kludgy
		elidx = element_to_idx(elname, elements);
		*(loc - 1) = ' '; // kludgy
		*s |= floornr << (4 * elidx + 2); // put this type of Gen on the floor
		// next occurrence:
		loc = strstr(loc + 1, genstr);
	}
	// process chips
	loc = strstr(line, chipstr);
	while (loc) {
		// move back until beginning of element name
		char* elname = loc;
		while (*(elname - 1) != ' ')
			--elname;
		*loc = '\0'; // kludgy
		elidx = element_to_idx(elname, elements);
		*loc = '-'; // kludgy
		*s |= floornr << (4 * elidx); // put this type of chip on the floor
		// next occurrence:
		loc = strstr(loc + 1, chipstr);
	}
}

int solve(uint32_t s, int nr_el) {
	int steps = 0;
	uint32_t nr_states = nr_states_from_elements(nr_el);
	uint32_t target = nr_states - 1; // solution: all things are on floor 3 --> all bits set

	// init
	bool* visited = calloc(nr_states, sizeof(bool)); // initialized to false
	uint32_t* parent = malloc(nr_states * sizeof(uint32_t)); // for backtracing after search
	// statetree[state] contains parent node for state with nr state
	for (uint32_t ii = 0; ii < nr_states; ++ii)
		parent[ii] = NO_STATE;
	struct state_queue* q = new_state_queue();

	// bfs
	state_queue_enqueue(q, s);
	visited[s] = true;
	bool found = false;
	while (!state_queue_isempty(q)) {
		uint32_t cur = state_queue_dequeue(q);
		if (cur == target) {
			found = true;
			break;
		}
		if (!is_safe_state(cur, nr_el))
			continue;
		// generate next states
		int floor = cur >> (4 * nr_el);
		for (int thing1 = 0; thing1 < 2*nr_el; ++thing1) {
			int shift1 = 2 * thing1; // lsb pos in state bitmap
			if (((cur >> shift1) & 0x03) != floor)
				continue; // thing1 not on this floor
			for (int dfloor = -1; dfloor <= 1; dfloor += 2) {
				int nextfloor = floor + dfloor;
				if (nextfloor < 0 || nextfloor > 3)
					continue;
				// move elevator pos
				uint32_t next0 = (cur & ~(0x03 << (4*nr_el))) | (nextfloor << (4*nr_el));
				// move thing1 pos
				uint32_t next1 = (next0 & ~(0x03 << shift1)) | (nextfloor << shift1);
				if (!visited[next1]) {
					state_queue_enqueue(q, next1);
					parent[next1] = cur;
					visited[next1] = true;
				}
				// bring second thing
				for (int thing2 = 0; thing2 < thing1; ++thing2) {
					int shift2 = 2 * thing2; // lsb pos in state bitmap
					if (((cur >> shift2) & 0x03) != floor)
						continue; // thing2 not on this floor
					uint32_t next2 = (next1 & ~(0x03 << shift2)) | (nextfloor << shift2);
					if (!visited[next2]) {
						state_queue_enqueue(q, next2);
						parent[next2] = cur;
						visited[next2] = true;
					}
				}
			}
		}
	}
	if (found) {
		// back tracing, counting steps
		steps = 0;
		uint32_t cur = target;
		while (cur != s) {
			++steps;
			cur = parent[cur];
		}
	}

	free(parent);
	destroy_state_queue(q);

	return steps;
}

int main(int argc, char* argv[]) {
	char* elements[MAX_ELEMENTS + 1]; // nul terminated char* array with element names
	elements[0] = NULL;
	uint32_t state = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		process_line(line, elements, &state);
	}

	// count elements
	int nr_elements = 0;
	while (elements[nr_elements])
		++nr_elements;
	nr_elements += 2; // 11B: 2 elements found, on floor 0

	show_state(state, nr_elements);

	// Solve puzzle
	int steps = solve(state, nr_elements);
	printf("%u\n", steps);

	// clean up
	free(line);
	for (int ii = 0; elements[ii] != NULL; ++ii)
		free(elements[ii]);
	return 0;
}
