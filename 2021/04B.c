#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 5

struct board {
	int           values[SIZE * SIZE];
	bool          marked[SIZE * SIZE];
	bool          won;
	struct board* next;
};

struct board* create_board() {
	struct board* b = malloc(sizeof(struct board));
	for (int ii = 0; ii < SIZE * SIZE; ++ii) {
		b->values[ii] = 0;
		b->marked[ii] = false;
	}
	b->won = false;
	b->next = false;
	return b;
}

void set_board_row(struct board* b, int row, char* str) {
	for (int ii = 0; ii < SIZE; ++ii)
		b->values[row * SIZE + ii] = strtol(str, &str, 10);
}

bool check_board(int v, struct board* b) {
	if (b->won)
		return true;
	// marks v on b, returns true when won
	for (int ii = 0; ii < SIZE * SIZE; ++ii) {
		if (b->values[ii] == v)
			b->marked[ii] = true;
	}
	for (int row = 0; row < SIZE; ++row) {
		bool full = true;
		for (int col = 0; full && col < SIZE; ++col)
			full = b->marked[row * SIZE + col] && full;
		if (full) {
			b->won = true;
			return true;
		}
	}
	for (int col = 0; col < SIZE; ++col) {
		bool full = true;
		for (int row = 0; full && row < SIZE; ++row)
			full = b->marked[row * SIZE + col] && full;
		if (full) {
			b->won = true;
			return true;
		}
	}
	return false;
}

int sum_board(struct board* b) {
	int sum = 0;
	for (int ii = 0; ii < SIZE * SIZE; ++ii) {
		if (!b->marked[ii])
			sum += b->values[ii];
	}
	return sum;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

int main(int argc, char* argv[]) {
	char* random_line = NULL;
	struct board* boards = NULL;

	char *line = NULL;
	size_t len = 0;
	int row_count = 0;
	int nr_boards = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		if (random_line == NULL) {
			int l = strlen(line) + 1;
			random_line = malloc(l * sizeof(char));
			memcpy(random_line, line, l);
		}
		else {
			if (row_count == 0) {
				struct board* b = create_board();
				b->next = boards;
				boards = b;
				++nr_boards;
			}
			set_board_row(boards, row_count, line);
			row_count = (row_count + 1) % SIZE;
		}
	}
	free(line);
	//  Play Bingo!
	char* s = random_line;
	int score = 0;
	while (nr_boards) {
		int nr = strtol(s, &s, 10);
		if (*s == ',')
			++s;
		struct board* b = boards;
		while (b) {
			if (!b->won && check_board(nr, b)) {
				--nr_boards;
				if (nr_boards == 0)
					score = sum_board(b) * nr;
			}
			b = b->next;
		}
		if (*s == 0 || *s == '\n')
			break;
	}
	printf("%d\n", score);

	// clean-up
	while (boards) {
		struct board* b = boards;
		boards = boards->next;
		free(b);
	}
	free(random_line);
	return 0;
}
