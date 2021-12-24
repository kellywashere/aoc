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
}

void add_move(struct move* movelist, int* nr_moves, int from, int to, int cost) {
	struct move move;
	movelist[*nr_moves].from = from;
	movelist[*nr_moves].to = to;
	movelist[*nr_moves].cost = cost;
	movelist[*nr_moves].is_home_move = to > 6;
	++(*nr_moves);
}

bool can_move_home(struct board* b, int from, int* pto) {
	int homecol = b->pos[from] - A;
	if (b->pos[homecol + 11] != EMPTY && b->pos[homecol + 11] != b->pos[from])
		return false;
	if (b->pos[homecol + 7] != EMPTY)
		return false;
	int to = (b->pos[homecol + 11] == EMPTY) ? homecol + 11 : homecol + 7;
	bool all_free = true;
	int ii;
	if (from < 7) { // from hall
		if (from <= homecol) { // move right
			for (ii = from + 1; all_free && ii <= homecol + 1; ++ii)
				all_free = b->pos[ii] == EMPTY;
		}
		else if (from >= homecol + 3) { // move left
		}
	}
}

void gen_move_list(struct board* b, stuct move* movelist, int* nr_moves) {
	// usually moves are single steps, except home moves
	*nr_moves = 0;
	int from, to;
	for (from = 0; from < 15; ++from) {
		if (g->pos[from] != EMPTY) {
			if (from < 7) { // hall
				if (can_move_home(b, from, &to)) {
				}
			}
		}
	}
}
01 2 3 4 56
  7 8 9 0
  1 2 3 4

/*
bool make_move(struct board* board, int from, int to) {
	if (from == to) return false;
	if (board->pos[from] == EMPTY) return false;
	if (board->pos[to] != EMPTY) return false;
	if (to > 6) { // move home
		int col = (to - 6) % 4;
		if (col != board->pos[from]

	}
	int a = from < to ? from : to;
	int b = from < to ? to : from;
	// b > a
	return true;
}
*/

struct board* read_board() {
	int ii, jj;
	char *line = NULL;
	size_t len = 0;

	struct board* b = malloc(sizeof(struct board));
	b->cost = 0;
	for (ii = 0; ii < (7 + 4 + 4); ++ii)
		b->pos[ii] = EMPTY;
	getline(&line, &len, stdin);
	getline(&line, &len, stdin);
	for (ii = 0; ii < 2; ++ii) {
		getline(&line, &len, stdin);
		for (jj = 0; jj < 4; ++jj)
			b->pos[7 + 4 * ii + jj] = line[2 * jj + 3] - 'A' + A;
	}
	free(line);
	return b;
}

int main(int argc, char* argv[]) {
	struct board* board = read_board();
	print_board(board);
	free(board);
	return 0;
}
