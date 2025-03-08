#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

typedef unsigned int uint;

#define MAX_INT64 0xFFFFFFFFFFFFFFFFULL

// EQUIVALENT C-CODE OF PROGRAM:
void run_program(uint64_t a) {
	uint b, c, d;
	for ( ; a != 0; a >>= 3) {
		b = a & 7;
		c = (a >> (b ^ 1)) & 7;
		d = b ^ c;
		printf("%c,", '0' + (d ^ 4));
	}
	printf("\n");
}

bool read_int(const char** pLine, int* x) {
	int num = 0;
	const char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

enum bit {
	ZERO = 0,
	ONE  = 1,
	UNKNOWN
};

struct undo_info { // for algo backtracking
	int      msbpos;
	enum bit bits[3];
};

void clear_all_bits(enum bit* bits, uint bits_sz) {
	for (uint ii = 0; ii < bits_sz; ++ii) {
		bits[ii] = UNKNOWN;
	}
}

bool can_mark_3_bits(enum bit* bits, int msbpos, uint val) {
	// NOTE: msbpos can be negative here!!
	for (int ii = 0; ii < 3; ++ii) {
		int bitv = (val >> (2 - ii)) & 1;
		if (msbpos + ii >= 0) {
			enum bit b = bits[msbpos + ii];
			if (b != UNKNOWN && b != bitv)
				return false;
		}
		else if (bitv == 1) {
			return false; // cannot set 1 at negative pos
		}
	}
	return true;
}

struct undo_info mark_3_bits(enum bit* bits, int msbpos, uint val) {
	// NOTE: msbpos can be negative here!!
	// assumption: already checked with can_mark_3_bits()
	struct undo_info undo;
	undo.msbpos = msbpos;
	for (int ii = 0; ii < 3; ++ii) {
		if (msbpos + ii >= 0) {
			undo.bits[ii] = bits[msbpos + ii];
			bits[msbpos + ii] = (val >> (2 - ii)) & 1;
		}
	}
	return undo;
}

void undo_bits(enum bit* bits, struct undo_info* undo) {
	for (int ii = 0; ii < 3; ++ii) {
		if (undo->msbpos + ii >= 0) {
			bits[undo->msbpos + ii] = undo->bits[ii];
		}
	}
}

uint64_t bits_to_uint64(enum bit* bits, uint bits_sz) {
	uint64_t retval = 0;
	for (uint ii = 0; ii < bits_sz; ++ii) {
		retval = (retval << 1) | (bits[ii] == ONE ? 1 : 0);
	}
	return retval;
}

void print_bits(enum bit* bits, uint bits_sz) {
	assert(bits_sz <= 64);
	for (uint ii = 0; ii < bits_sz; ++ii) {
		enum bit b = bits[ii];
		printf("%c", b == ZERO ? '0' : b == ONE ? '1' : '.');
	}
	printf("\n");
}

// Recursive solution finder:
uint64_t find_bits_rec(enum bit* bits, uint bits_sz, int msbpos,
		uint* target_d, uint target_d_sz, uint64_t lowest_found_so_far) {
	// we try to deduce value of A by going in reverse, starting at last output byte
	// because we work in reverse, we number bits opposite to normal: 0 is MSB, 1 is MSB-1 etc

	// recursion base case
	if (target_d_sz == 0) {
		uint64_t bval = bits_to_uint64(bits, bits_sz);
		printf("Found solution %" PRIu64 ": ", bval);
		print_bits(bits, bits_sz);
		return bval < lowest_found_so_far ? bval : lowest_found_so_far;
	}

	uint d = target_d[target_d_sz - 1]; // last value
	int msbpos_b = msbpos;
	for (uint b = 0; b <= 7; ++b) {
		int msbpos_c = msbpos_b - (b ^ 1);
		uint c = b ^ d; // d = b ^ c => c = b ^ d
		//printf("d = %d, b@%d = %d ==> c@%d = %d\n", d, msbpos_b, b, msbpos_c, c);
		if (!can_mark_3_bits(bits, msbpos_b, b)) {
			//printf("Cannot write b = %d at pos %d\n", b, msbpos_b);
			continue;
		}
		struct undo_info undo_b = mark_3_bits(bits, msbpos_b, b);

		if (!can_mark_3_bits(bits, msbpos_c, c)) {
			//printf("Cannot write c = %d at pos %d\n", c, msbpos_c);
			undo_bits(bits, &undo_b);
			continue;
		}
		struct undo_info undo_c = mark_3_bits(bits, msbpos_c, c);
		//print_bits(bits, bits_sz);

		lowest_found_so_far = find_bits_rec(bits, bits_sz, msbpos + 3, target_d, target_d_sz - 1, lowest_found_so_far);

		undo_bits(bits, &undo_c);
		undo_bits(bits, &undo_b);
	}

	return lowest_found_so_far;
}


uint64_t find_bits(uint* target_d, uint target_d_sz) {
	uint bits_sz = target_d_sz * 3;
	enum bit* bits = malloc(bits_sz * sizeof(enum bit));
	clear_all_bits(bits, bits_sz);

	uint64_t val = find_bits_rec(bits, bits_sz, 0, target_d, target_d_sz, MAX_INT64);

	free(bits);

	return val;
}

int main(int argc, char* argv[]) {
	// First, read program code from stdin
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1 && strncmp(line, "Program:", 8) != 0)
		;
	const char* l = line;

	// We are now at line that starts with Program: (no error checking here...)
	uint target_out[20];
	uint out_sz = 0;

	int x;
	while (read_int(&l, &x))
		target_out[out_sz++] = x;
	assert(out_sz <= 20);

	uint target_d[20]; // d = b ^ c; we output d ^ 4
	for (uint ii = 0; ii < out_sz; ++ii)
		target_d[ii] = target_out[ii] ^ 4;

	uint64_t a = find_bits(target_d, out_sz);
	printf("%" PRIu64 "\n", a);

	//run_program(a); // test if value indeed works

	return 0;
}
