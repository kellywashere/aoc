#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>

#define MAXFIELDS  32
#define MAXTICKETS 256

struct range {
	int from;
	int to; // inclusive
};

struct field_spec {
	bool         has_departure; // has departure in name
	struct range range[2]; // each range line contains 2 ranges
};

struct ticket {
	int val[MAXFIELDS];
};

void print_range(struct range* r) {
	printf("%d-%d", r->from, r->to);
}

void print_field_spec(struct field_spec* fs) {
	print_range(&fs->range[0]);
	printf(" or ");
	print_range(&fs->range[1]);
	printf("\n");
}

bool is_valid(int n, struct field_spec* specs, int specs_sz) {
	for (int ii = 0; ii < specs_sz; ++ii) {
		for (int jj = 0; jj < 2; ++jj)
			if (n >= specs[ii].range[jj].from && n <= specs[ii].range[jj].to)
				return true;
	}
	return false;
}

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_int(char** pLine, int* x) {
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
	*x = num;
	return true;
}

void print_puzzle_array(bool array[][MAXFIELDS], int nr_fields) {
	printf("\nPuzzle array:\n");
	for (int tnr_idx = 0; tnr_idx < nr_fields; ++tnr_idx) {
		for (int fld_idx = 0; fld_idx < nr_fields; ++fld_idx) {
			printf("%c ", array[tnr_idx][fld_idx] ? 'X' : '.');
		}
		printf("\n");
	}
}

void solve_puzzle_array(bool array[][MAXFIELDS], int nr_fields) {
	// each row that has only one `true` (at idx `idx`): set all other row at idx to false
	bool changed;
	do {
		changed = false;
		for (int r = 0; r < nr_fields; ++r) {
			int cc;
			int cnt = 0;
			for (int c = 0; c < nr_fields; ++c) {
				if (array[r][c]) {
					++cnt;
					cc = c;
				}
			}
			if (cnt == 1) {
				for (int rr = 0; rr < nr_fields; ++rr) {
					if (rr != r && array[rr][cc]) {
						array[rr][cc] = false;
						changed = true;
					}
				}
			}
		}
	} while (changed);
	// We do not even have to resort to other things: this solves it...
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct field_spec specs[MAXFIELDS];
	int nr_fields = 0;
	// read ranges
	while (getline(&line, &len, stdin) != -1 && !empty_line(line)) {;
		char* l = line;
		read_int(&l, &specs[nr_fields].range[0].from);
		read_int(&l, &specs[nr_fields].range[0].to);
		read_int(&l, &specs[nr_fields].range[1].from);
		read_int(&l, &specs[nr_fields].range[1].to);
		specs[nr_fields].has_departure = strncmp(line, "departure", 9) == 0;
		++nr_fields;
	}

	// TODO: read my ticket
	int myticket[MAXFIELDS];
	while (getline(&line, &len, stdin) != -1 && !strstr(line, "your"));
	getline(&line, &len, stdin);
	char* l = line;
	for (int ii = 0; ii < nr_fields; ++ii)
		read_int(&l, &myticket[ii]);

	// find nearby tickets
	while (getline(&line, &len, stdin) != -1 && !strstr(line, "nearby"))
		;

	int tickets[MAXTICKETS][MAXFIELDS];
	int nr_tickets = 0;

	while (getline(&line, &len, stdin) != -1) {
		l = line;
		bool valid = true;
		for (int ii = 0; ii < nr_fields; ++ii) {
			int n;
			read_int(&l, &n);
			tickets[nr_tickets][ii] = n;
			if (!is_valid(n, specs, nr_fields))
				valid = false;
		}
		if (valid)
			++nr_tickets;
	}

	bool matches[MAXFIELDS][MAXFIELDS] = {0};
	// matches[x][y] is true if the xth number in ticket can be spec nr y
	for (int tnr_idx = 0; tnr_idx < nr_fields; ++tnr_idx) {
		for (int fld_idx = 0; fld_idx < nr_fields; ++fld_idx) {
			bool works = true;
			for (int t_idx = 0; works && t_idx < nr_tickets; ++t_idx) {
				int v = tickets[t_idx][tnr_idx];
				if (!(v >= specs[fld_idx].range[0].from && v <= specs[fld_idx].range[0].to) &&
						!(v >= specs[fld_idx].range[1].from && v <= specs[fld_idx].range[1].to))
					works = false;
			}
			if (works)
				matches[tnr_idx][fld_idx] = true;
		}
	}

	//print_puzzle_array(matches, nr_fields);
	solve_puzzle_array(matches, nr_fields);
	//print_puzzle_array(matches, nr_fields);

	uint64_t prod = 1;
	for (int tnr_idx = 0; tnr_idx < nr_fields; ++tnr_idx) {
		int cnt = 0;
		int idx = 0;
		for (int fld_idx = 0; fld_idx < nr_fields; ++fld_idx) {
			if (matches[tnr_idx][fld_idx]) {
				++cnt;
				idx = fld_idx;
			}
		}
		if (cnt != 1)
			printf("Sorry, puzzle is not solved yet!\n");
		if (specs[idx].has_departure)
			prod *= myticket[tnr_idx];
	}
	printf("%"PRIu64"\n", prod);


	free(line);
	return 0;
}
