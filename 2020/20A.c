#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define TILESIZE 10

#define MAX_NRTILES 256

// operations, multiple ops are done LSB first
#define FLIPUD 1
#define FLIPLR 2
#define ROTRIGHT 4

// doing: nothing, flipud, fliplr, flipud, rot, flipud, fliplr, flipud you get all configs
const int op_seq[] = {0, 1, 2, 1, 4, 1, 2, 1};
const int op_eqv[] = {0, 1, 3, 2, 6, 4, 5, 7}; // bitcoding to achieve states in op_seq starting from init

struct tile {
	int id;
	int row[TILESIZE]; // #bits: TILESIZE
	/*
	int match_id[4];  // ids that match right, top, left and bot respectively
	int match_ops[4]; // bit-coded
	*/
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int read_row(char* l) {
	int x = 0;
	for (int ii = 0; ii < TILESIZE; ++ii)
		x = (x << 1) | (l[ii] == '#' ? 1 : 0);
	return x;
}

void print_row(int x) {
	for (int ii = TILESIZE - 1; ii >= 0; --ii) {
		int b = (x >> ii) & 1;
		printf("%c", b ? '#' : '.');
	}
	printf(" (%d)\n", x);
}

void print_tile(struct tile* t) {
	printf("Tile %d:\n", t->id);
	for (int row = 0; row < TILESIZE; ++row)
		print_row(t->row[row]);
}

void flipud(struct tile* t) {
	int r2 = TILESIZE - 1;
	for (int r = 0; r < TILESIZE / 2; ++r, --r2) {
		int x = t->row[r];
		t->row[r] = t->row[r2];
		t->row[r2] = x;
	}
}

void fliplr(struct tile* t) {
	for (int r = 0; r < TILESIZE; ++r) {
		int x = t->row[r];
		t->row[r] = 0;
		for (int ii = 0; ii < TILESIZE; ++ii) {
			t->row[r] = (t->row[r] << 1) | (x & 1);
			x >>= 1;
		}
	}
}

void rotright(struct tile* t) {
	int cpy[TILESIZE];
	memcpy(cpy, t->row, TILESIZE * sizeof(int));
	for (int r = 0; r < TILESIZE; ++r) {
		t->row[r] = 0;
		int bpos = TILESIZE - r - 1;
		for (int ii = 0; ii < TILESIZE; ++ii)
			t->row[r] |= ((cpy[ii] >> bpos) & 1) << ii;
	}
}

void do_operation(struct tile* t, int op) {
	if ((op & FLIPUD))
		flipud(t);
	if ((op & FLIPLR))
		fliplr(t);
	if ((op & ROTRIGHT))
		rotright(t);
}


void tile_cpy(struct tile* dst, struct tile* src) {
	memcpy(dst, src, sizeof(struct tile));
}

bool match_top(struct tile* t1, struct tile* t2, int* match_op) {
	// does t2 fit to the top row of t1?
	struct tile cpy;
	tile_cpy(&cpy, t2);
	for (int ii = 0; ii < 8; ++ii) {
		do_operation(&cpy, op_seq[ii]);
		if (cpy.row[TILESIZE - 1] == t1->row[0]) {
			*match_op = op_eqv[ii];
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct tile tiles[MAX_NRTILES];
	int nr_tiles = 0;

	int rows_read = 0;

	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line))
			continue;
		if (l[0] == 'T')
			tiles[nr_tiles].id = read_int(&l);
		else {
			tiles[nr_tiles].row[rows_read++] = read_row(l);
			if (rows_read == TILESIZE) {
				++nr_tiles;
				rows_read = 0;
			}
		}
	}
	free(line);

	// for each tile: check how many other tiles it can connect to
	// corner tiles have only 2 matches
	uint64_t prod = 1;
	for (int t1_idx = 0; t1_idx < nr_tiles; ++t1_idx) {
		int count_matching_sides = 0;
		for (int side = 0; side < 4; ++side) {
			int match_op; //(dummy)
			for (int t2_idx = 0; t2_idx < nr_tiles; ++t2_idx) {
				if (t1_idx == t2_idx)
					continue;
				if (match_top(&tiles[t1_idx], &tiles[t2_idx], &match_op)) {
					++count_matching_sides;
					break; // !!
				}
			}
			rotright(&tiles[t1_idx]);
		}
		if (count_matching_sides == 2) // corner
			prod *= tiles[t1_idx].id;
		// printf("Tile %d (ID: %d) has %d matching sides\n", t1_idx, tiles[t1_idx].id, count_matching_sides[t1_idx]);
	}
	printf("%"PRIu64"\n", prod);

	return 0;
}
