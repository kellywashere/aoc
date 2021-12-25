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
	int hash;
	/*
01 2 3 4 56
..x.x.x.x..
  7 8 9 0
  1 2 3 4
  */
};

struct move {
	int  from;
	int  to;
	int  cost;
	bool is_home_move;
};

uint32_t hash_board(struct board* b) { // no risk of collision, low nr of possible boards
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
	uint32_t hash = 0;
	for (ii = 0; ii < 8; ++ii) {
		hash = (hash << 4) | (pos_of_pods[ii] - 1);
	}
	return hash;
}

void print_board(struct board* b) {
	int ii, jj;
	printf("%c", pods2char[b->pos[0]]);
	for (ii = 0; ii < 4; ++ii)
		printf("%cx", pods2char[b->pos[ii + 1]]);
	printf("%c%c\n", pods2char[b->pos[5]], pods2char[b->pos[6]]);
	for (ii = 0; ii < 2; ++ii) {
		printf(" ");
		for (jj = 0; jj < 4; ++jj)
			printf(" %c", pods2char[b->pos[7 + ii * 4 + jj]]);
		printf("\n");
	}
	printf("Cost: %d\n", b->cost);
}

/*
void add_move(struct move* movelist, int* nr_moves, int from, int to, int cost) {
	struct move move;
	movelist[*nr_moves].from = from;
	movelist[*nr_moves].to = to;
	movelist[*nr_moves].cost = cost;
	movelist[*nr_moves].is_home_move = to > 6;
	++(*nr_moves);
}
*/

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
	if (from >= to) {
		t = from;
		from = to;
		to = t;
	} // now from < to
	int steps = 0;
	if (!range(b, &from, &steps))
		return false;
	if (!range(b, &to, &steps))
		return false;
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

/*
void gen_move_list(struct board* b, struct move* movelist, int* nr_moves) {
	// usually moves are single steps, except home moves
	*nr_moves = 0;
	int from, to;
	for (from = 0; from < 15; ++from) {
		if (b->pos[from] != EMPTY) {
		}
	}
}
*/

void make_move(struct board* b, int from, int to, int steps) {
	// no error checking is done here
	// steps calc is done elsewhere
	b->pos[to] = b->pos[from];
	b->pos[from] = EMPTY;
	b->cost += steps * movecost[b->pos[to] - A];
	b->hash = hash_board(b);
}

bool all_home(struct board* b) {
	int homecol;
	bool allhome = true;
	for (homecol = 0; allhome && homecol < 4; ++homecol)
		allhome = (b->pos[homecol + 7] == A + homecol) && (b->pos[homecol + 11] == A + homecol);
	return allhome;
}

struct board* read_board(FILE* fp) {
	int ii, jj;
	char *line = NULL;
	size_t len = 0;

	struct board* b = malloc(sizeof(struct board));
	b->cost = 0;
	for (ii = 0; ii < (7 + 4 + 4); ++ii)
		b->pos[ii] = EMPTY;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	for (ii = 0; ii < 2; ++ii) {
		getline(&line, &len, fp);
		for (jj = 0; jj < 4; ++jj)
			b->pos[7 + 4 * ii + jj] = line[2 * jj + 3] - 'A' + A;
	}
	free(line);
	return b;
}

int main(int argc, char* argv[]) {
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
	int from, to;
	while (!all_home(board)) {
		print_board(board);
		printf("\n01.2.3.4.56\n");
		printf("  7 8 9 0\n");
		printf("  1 2 3 4\n");
		printf("\nFrom: \n");
		int steps = 0;
		while (!steps) {
			scanf("%d %d", &from, &to);
			if (from < 0 || from > 14 || to < 0 || to > 14)
				printf("out of range\n");
			else if (to >= 7)
				printf("Home moves done automatically\n");
			else if (board->pos[from] == EMPTY)
				printf("Can't move from empty position\n");
			else if (board->pos[to] != EMPTY)
				printf("Can't move to non-empty position\n");
			else {
				steps = calc_steps(board, from, to);
				if (steps == 0)
					printf("invalid move\n");
			}
		}
		make_move(board, from, to, steps);
		bool homemove_made;
		do {
			homemove_made = false;
			for (from = 0; from <= 14; ++from) {
				if (can_move_home(board, from, &to, &steps)) {
					printf("Moving home: %d to %d\n", from, to);
					make_move(board, from, to, steps);
					homemove_made = true;
				}
			}
		} while (homemove_made);
	}
	print_board(board);

	free(board);
	return 0;
}
