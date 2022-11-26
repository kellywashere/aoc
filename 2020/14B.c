#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define NBITS 36

uint64_t read_uint64(char** pLine) {
	uint64_t num = 0;
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

struct mask {
	uint64_t one_mask;   // ORed with value
	uint64_t float_mask; // a 1 means X
};

struct mem_addr {
	uint64_t addr;       // only relevant where float_mask is 0
	uint64_t float_mask; // a 1 means X
	uint64_t value;
};

void reset_mask(struct mask* mask) {
	mask->one_mask = 0;
	mask->float_mask = 0;
}

void reset_mem_addr(struct mem_addr* mem) {
	mem->addr = 0;
	mem->float_mask = 0;
	mem->value = 0;
}

void set_mask(struct mask* mask, char* l) {
	reset_mask(mask);
	char* start = l;
	// find first valid char
	while (*l && *l != 'X' && *l != '0' && *l != '1')
		++l;
	// find first non-valid char
	while (*l == 'X' || *l == '0' || *l == '1')
		++l;
	// now go backwards
	--l;
	uint64_t b = 1; // bit pos
	while (l >= start && (*l == 'X' || *l == '0' || *l == '1')) {
		if (*l == '1')
			mask->one_mask |= b;
		else if (*l == 'X')
			mask->float_mask |= b;
		b <<= 1;
		--l;
	}
}

void apply_mask(struct mask* mask, struct mem_addr* mem) {
	mem->addr |= mask->one_mask;
	mem->float_mask = mask->float_mask;
}

void print_mem_addr(struct mem_addr* mem) {
	for (int b = NBITS - 1; b >= 0; --b) {
		uint64_t bm = (1ULL << b);
		char c = '0';
		if ( (mem->float_mask & bm) )
			c = '-';
		else if ( (mem->addr & bm) )
			c = '1';
		printf("%c", c);
	}
	printf(" = %"PRIu64"\n", mem->value);
}

bool compare_non_float(struct mem_addr* m1, struct mem_addr* m2) {
	uint64_t bm = 1;
	for (int b = 0; b < NBITS; ++b) {
		if ((m1->float_mask & bm) == 0 && (m2->float_mask & bm) == 0 && (m1->addr & bm) != (m2->addr & bm))
			return false;
		bm <<= 1;
	}
	return true;
}

bool matches(struct mem_addr* m1, struct mem_addr* m2, int floatval) {
	// all bits that are fixed in same pos and are non-float are already known to match here
	// here we try: substituting floatval in floating positions of m1, check if match to m2
	// returns true when match
	// done becomes true when this is the highest value of floatval possible, given nr of bits
	uint64_t bm = 1;
	for (int b = 0; b < NBITS; ++b) {
		if ( (m1->float_mask & bm) ) {
			int bval = floatval & 1;
			floatval >>= 1;
			//bool float2 = (m2->float_mask & bm) != 0;
			if ( (m2->float_mask & bm) == 0 && ((m2->addr >> b) & 1) != bval)
				return false;
		}
		bm <<= 1;
	}
	return true;
}

int popcount(uint64_t x) {
	int pc = 0;
	while (x) {
		pc += (x & 1) == 1 ? 1 : 0;
		x >>= 1;
	}
	return pc;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct mem_addr mem[1024];
	int mem_sz = 0;
	struct mask mask;

	while (getline(&line, &len, stdin) != -1) {
		if (strstr(line, "mask")) {
			set_mask(&mask, line + 7);
			// printf("%s", line);
		}
		else if (strstr(line, "mem")) {
			char* l = line;
			uint64_t addr = read_uint64(&l);
			uint64_t val = read_uint64(&l);
			reset_mem_addr(mem + mem_sz);
			mem[mem_sz].addr = addr;
			mem[mem_sz].value = val;
			apply_mask(&mask, mem + mem_sz);
			// print_mem_addr(mem + mem_sz);
			++mem_sz;
		}
	}

	// count max nr of floating wires, for mem reservation
	int max_floats = 0;
	for (int ii = 0; ii < mem_sz - 1; ++ii) {
		int popcnt = popcount(mem[ii].float_mask);
		max_floats = popcnt > max_floats ? popcnt : max_floats;
	}
	// reserve memory
	int overwritten_cap = (1 << max_floats); // 2^max_floats
	bool* overwritten = malloc(overwritten_cap * sizeof(bool));

	// For each mem write: check which following writes can overwrite stuff
	uint64_t sum = 0;
	for (int ii = 0; ii < mem_sz; ++ii) {
		int nrfloat = popcount(mem[ii].float_mask);
		int maxfloatval = (1 << nrfloat) - 1;
		memset(overwritten, 0, (maxfloatval + 1) * sizeof(bool)); // reset to nothing overwritten
		for (int jj = ii + 1; jj < mem_sz; ++jj) {
			if (compare_non_float(mem + ii, mem + jj)) { // we have equal fixed addr bits
				for (int floatval = 0; floatval <= maxfloatval; ++floatval) {
					if (matches(mem + ii, mem + jj, floatval))
						overwritten[floatval] = true;
				}
			}
		}
		// now count not-overwritten mem addresses
		int count = 0;
		for (int jj = 0; jj <= maxfloatval; ++jj)
			count += overwritten[jj] ? 0 : 1;
		sum += count * mem[ii].value;
	}
	printf("%"PRIu64"\n", sum);

	free(line);
	return 0;
}
