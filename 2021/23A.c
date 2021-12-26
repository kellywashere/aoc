#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#define MAX_MOVES 100 /* max nr of moves from a given position */

enum pods {
	EMPTY,
	A,
	B,
	C,
	D
};

char* pods2char = ".ABCD";
int movecost[] = {1, 10, 100, 1000};

struct board {
	enum pods pos[7 + 4 + 4]; // hall: not counting positions above homes
	int cost;
	uint32_t hash; // non-unique zobrist hash
	/*
01.2.3.4.56
  7 8 9 0
  1 2 3 4
  */
};

struct hash_info {
	uint32_t board_id;
	uint32_t cost_from_here;
};

#define HASH_BITS 20
uint32_t* g_zobrist = NULL; // zobrist hashing numbers
uint32_t g_final_hash; // hash of solved board
struct hash_info* g_hash_table = NULL;

uint32_t id_board(struct board* b) { // UNIQUE board id
	// first 4 bits: pos of A, second 4 bits: pos of A (>first 4 bits), 3rd 4 bits: pos of B, etc
	int pos_of_pods[8] = {0};
	int ii;
	for (ii = 0; ii < (7 + 4 + 4); ++ii) {
		if (b->pos[ii] != EMPTY) {
			int idx = 2 * (b->pos[ii] - A);
			if (pos_of_pods[idx] == 0)
				pos_of_pods[idx] = ii + 1;
			else
				pos_of_pods[idx + 1] = ii + 1;
		}
	}
	uint32_t id = 0;
	for (ii = 0; ii < 8; ++ii) {
		id = (id << 4) | (pos_of_pods[ii] - 1);
	}
	return id;
}

void zobhash_board(struct board* b) {
	uint32_t hash = 0;
	int ii;
	for (ii = 0; ii < 15; ++ii)
		hash ^= g_zobrist[5 * ii + b->pos[ii]];
	b->hash = hash;
}

void calc_zobhash_final() {
	uint32_t hash = 0;
	int ii;
	for (ii = 0; ii < 7; ++ii)
		hash ^= g_zobrist[5 * ii + EMPTY];
	for (ii = 7; ii < 15; ++ii)
		hash^= g_zobrist[5 * ii + 1 + (ii - 7) % 4];
	g_final_hash = hash;
}

void gen_zobrist_nrs() {
	g_zobrist = malloc(15 * 5 * sizeof(uint32_t));
	int ii = 0;
	enum pods jj;
	for (ii = 0; ii < 15; ++ii)
		for (jj = EMPTY; jj <= D; ++jj)
			g_zobrist[ii * 5 + jj] = rand() & ((1 << HASH_BITS) - 1);
}

void print_board(struct board* b) {
	int ii, jj;

	printf("#############\n");
	printf("#%c", pods2char[b->pos[0]]);
	for (ii = 0; ii < 4; ++ii)
		printf("%c.", pods2char[b->pos[ii + 1]]);
	printf("%c%c#\n", pods2char[b->pos[5]], pods2char[b->pos[6]]);

	printf("###");
	for (jj = 0; jj < 4; ++jj)
		printf("%c#", pods2char[b->pos[7 + jj]]);
	printf("##\n");

	printf("  #");
	for (jj = 0; jj < 4; ++jj)
		printf("%c#", pods2char[b->pos[11 + jj]]);
	printf("\n");

	printf("  #########\n");

	printf("Cost: %d\n", b->cost);
	printf("HASH: %08X\n\n", b->hash);
}

bool range(struct board* b, int* tf, int* steps) {
	// brings tf in range, calcs steps to do so
	if (*tf >= 11) {
		if (b->pos[*tf - 4] != EMPTY)
			return false;
		*tf -= 4;
		++(*steps);
		return true;
	}
	if (*tf == 0) {
		if (b->pos[1] != EMPTY)
			return false;
		++(*steps);
		*tf = 1;
	}
	if (*tf == 6) {
		if (b->pos[5] != EMPTY)
			return false;
		++(*steps);
		*tf = 5;
	}
	return true;
}

int calc_steps(struct board* b, int from, int to) {
	// includes error checking: returns 0 on error
	if (b->pos[to] != EMPTY)
		return 0;
	if (b->pos[from] == EMPTY)
		return 0;
	int t;
	int steps = 0;
	if (!range(b, &from, &steps))
		return false;
	if (!range(b, &to, &steps))
		return false;
	if (from >= to) {
		t = from;
		from = to;
		to = t;
	} // now from < to
	if (from >= 7) { // home to home
		for (t = from - 5; t <= to - 6; ++t)
			if (b->pos[t] != EMPTY)
				return 0;
		steps += 2 * (to - from) + 2;
	}
	else if (to >= 7) { // hall to home
		if (from <= to - 6) { // move right
			for (t = from + 1; t <= to - 6; ++t)
				if (b->pos[t] != EMPTY)
					return 0;
			steps += 2 * (to - from) - 10;
		}
		else { // move left
			for (t = from - 1; t >= to - 5; --t)
				if (b->pos[t] != EMPTY)
					return 0;
			steps += 2 * (from - to) + 12;
		}
	}
	else { // hall to hall
		for (t = from + 1; t <= to - 1; ++t)
			if (b->pos[t] != EMPTY)
				return 0;
		steps += 2 * (to - from);
	}
	return steps;
}

bool can_move_home(struct board* b, int from, int* pto, int* psteps) {
	int homecol = b->pos[from] - A;
	if (from >= 11 && homecol == from - 11) // already home
		return false;
	if (b->pos[homecol + 11] != EMPTY && b->pos[homecol + 11] != b->pos[from])
		return false;
	int to = (b->pos[homecol + 11] == EMPTY) ? homecol + 11 : homecol + 7;
	int steps = calc_steps(b, from, to);
	if (!steps)
		return false;
	*pto = to;
	*psteps = steps;
	return true;
}

void make_move(struct board* b, int from, int to, int steps) {
	// no error checking is done here
	// steps calc is done elsewhere
	b->pos[to] = b->pos[from];
	b->pos[from] = EMPTY;
	b->cost += steps * movecost[b->pos[to] - A];
	zobhash_board(b);
}

void make_home_moves(struct board* b, bool verbose) {
	bool homemove_made;
	int from, to, steps;
	do {
		homemove_made = false;
		for (from = 0; from <= 14; ++from) {
			if (can_move_home(b, from, &to, &steps)) {
				make_move(b, from, to, steps);
				homemove_made = true;
			}
		}
	} while (homemove_made);
}

bool all_home(struct board* b) {
	if (b->hash != g_final_hash)
		return false;
	return id_board(b) == 0x7B8C9DAE;
	/*
	int homecol;
	bool allhome = true;
	for (homecol = 0; allhome && homecol < 4; ++homecol)
		allhome = (b->pos[homecol + 7] == A + homecol) && (b->pos[homecol + 11] == A + homecol);
	return allhome;
	*/
}

void copy_board(struct board* dest, struct board* src) {
	memcpy(dest, src, sizeof(struct board));
}

enum pods parse_char(char c) {
	return c == '.' ? EMPTY : c - 'A' + A;
}

struct board* read_board(FILE* fp) {
	int ii, jj;
	char *line = NULL;
	size_t len = 0;

	struct board* b = malloc(sizeof(struct board));
	b->cost = 0;
	for (ii = 0; ii < (7 + 4 + 4); ++ii)
		b->pos[ii] = EMPTY;
	getline(&line, &len, fp); // #############
	getline(&line, &len, fp);
	b->pos[0] = parse_char(line[1]);
	for (ii = 0; ii < 5; ++ii)
		b->pos[1 + ii] = parse_char(line[2 + 2 * ii]);
	b->pos[6] = parse_char(line[11]);
	for (ii = 0; ii < 2; ++ii) {
		getline(&line, &len, fp);
		for (jj = 0; jj < 4; ++jj)
			b->pos[7 + 4 * ii + jj] = parse_char(line[2 * jj + 3]);
	}
	free(line);
	zobhash_board(b);
	return b;
}

#define BOARDS 250

bool try_move(struct board* boards, int nr_boards, int from, int to, int lowest_cost_of_all) {
	struct board* curboard = &boards[nr_boards - 1];
	struct board* newboard = &boards[nr_boards];
	int steps = calc_steps(curboard, from, to);
	if (!steps)
		return false;

	copy_board(newboard, curboard);
	make_move(newboard, from, to, steps);
	make_home_moves(newboard, false);

	if (lowest_cost_of_all > 0 && newboard->cost > lowest_cost_of_all)
		return false;
	// TODO: next part is clumsy, hash table for visited?
	for (int ii = 0; ii < nr_boards; ++ii)
		if (boards[ii].hash == newboard->hash)
			if (id_board(&boards[ii]) == id_board(newboard))
				return false;
	return true;
}

int solve_board(struct board* boards, int nr_boards, int* lowest_cost_of_all) {
	// returns lowest cost to solve boards[nr_boards - 1]
	struct board* curboard = &boards[nr_boards - 1];
	struct board* newboard = &boards[nr_boards];
	if (all_home(curboard)) {
		/*
		printf("Solution found with cost %d\n", curboard->cost);
		for (int ii = 0; ii < nr_boards; ++ii)
			print_board(&boards[ii]);
		*/

		if (*lowest_cost_of_all == 0 || curboard->cost < *lowest_cost_of_all)
			*lowest_cost_of_all = curboard->cost;
		return 0;
	}
	if (g_hash_table[curboard->hash].board_id) {
		if (id_board(curboard) == g_hash_table[curboard->hash].board_id) {
			int cfh = g_hash_table[curboard->hash].cost_from_here;
			if (cfh == -1) {
				printf("Unsolvable from here:\n");
				print_board(curboard);
			}
			else {
				int tot_cost = curboard->cost + cfh;
				/*
				printf("Hashed solution found with cost %d + %d = %d\n", curboard->cost, tot_cost - curboard->cost,  tot_cost);
				for (int ii = 0; ii < nr_boards; ++ii)
					print_board(&boards[ii]);
				*/

				if (*lowest_cost_of_all == 0 || tot_cost < *lowest_cost_of_all)
					*lowest_cost_of_all = tot_cost;
			}
			return g_hash_table[curboard->hash].cost_from_here;
		}
	}
	// find possible moves
	int from;
	int solve_cost = -1; // lowest found cost to solve, -1 if no sol
	int c = 0;
	for (from = 0; from <= 14; ++from) {
		if (curboard->pos[from] != EMPTY) {
			if (from < 7) { // hall
				if (from > 0 && try_move(boards, nr_boards, from, from - 1, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
				if (from < 6 && try_move(boards, nr_boards, from, from + 1, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
			}
			else if (from < 11) { // home pos, top row
				if (try_move(boards, nr_boards, from, from - 6, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
				if (try_move(boards, nr_boards, from, from - 5, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
			}
			else if (curboard->pos[from - 4] == EMPTY) { // home pos, bot row
				if (try_move(boards, nr_boards, from, from - 10, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
				if (try_move(boards, nr_boards, from, from - 9, *lowest_cost_of_all)) {
					c = solve_board(boards, nr_boards + 1, lowest_cost_of_all);
					if (c >= 0) {
						c += newboard->cost - curboard->cost; // add cost to go to newboard
						if (solve_cost == -1 || c < solve_cost)
							solve_cost = c;
					}
				}
			}
		}
	}
	if (solve_cost != -1) { // Problem: due to pruning in try_move, we cannot distinguish between unsolvable and pruned
		g_hash_table[curboard->hash].board_id = id_board(curboard);
		g_hash_table[curboard->hash].cost_from_here = solve_cost;
		int tot_cost = curboard->cost + solve_cost;
		if (solve_cost != -1 && (*lowest_cost_of_all == 0 || tot_cost < *lowest_cost_of_all))
			*lowest_cost_of_all = tot_cost;
	}
	return solve_cost;
}

void make_debug_move(struct board* b, int from, int to) {
	int steps = calc_steps(b, from, to);
	if (steps) {
		make_move(b, from, to, steps);
		make_home_moves(b, false);
	}
}

int main(int argc, char* argv[]) {
	// Init hash stuff
	gen_zobrist_nrs();
	calc_zobhash_final();

	g_hash_table = calloc(1 << HASH_BITS, sizeof(struct hash_info));

	if (argc < 2) {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		return 1;
	}
	FILE* fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "Could not open file %s\n", argv[1]);
		return 1;
	}
	struct board* board = read_board(fp);


	// make_debug_move(board, 9, 2);
	// make_debug_move(board, 12, 3);

	// make_debug_move(board, 10, 4);
	// make_debug_move(board, 14, 5);





	struct board* boards = malloc(BOARDS * sizeof(struct board));
	copy_board(&boards[0], board);
	free(board);

	int lowest_cost_of_all = 0;
	int solve_cost = boards[0].cost + solve_board(boards, 1, &lowest_cost_of_all);
	printf("%d\n", solve_cost);
	printf("%d\n", lowest_cost_of_all);

	free(boards);
	return 0;
}
