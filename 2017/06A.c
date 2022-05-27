#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

int redistribute(int* b, int nrbanks) {
	// find biggest
	int maxidx = 0;
	for (int ii = 1; ii < nrbanks; ++ii)
		maxidx = b[ii] > b[maxidx] ? ii : maxidx;
	// redist
	int m = b[maxidx];
	b[maxidx] = 0;
	for (int ii = 0; ii < m; ++ii)
		++b[(maxidx + ii + 1) % nrbanks];
	return maxidx; // returns index of biggest, for analysis purposes
}


#define NRHASHBITS 28
#define HASHMASK ((1<<NRHASHBITS)-1)

uint32_t rothash(uint32_t v, int amount) {
	if (amount == 0)
		return v;
	if (amount > 0) {
		uint32_t v1 = (v << amount); // amount zeros at LSB size
		uint32_t v2 = (v >> (NRHASHBITS-amount));
		return (v1 | v2) & HASHMASK;
	}
	else {
		amount = -amount;
		uint32_t v1 = (v >> amount);
		uint32_t v2 = (v << (NRHASHBITS-amount));
		return (v1 | v2) & HASHMASK;
	}
}

uint32_t gen_hash(int* b, int nrbanks, uint32_t* hashvals) {
	uint32_t hash = 0;
	for (int ii = 0; ii < nrbanks; ++ii)
		hash = hash ^ rothash(hashvals[ii], b[ii]);
	return hash;
}

int main(int argc, char* argv[]) {
	int x;
	int b[32]; // max nr of banks foreseen: 32
	int nrbanks = 0; // banks used
	while (scanf("%d", &x) == 1)
		b[nrbanks++] = x;

	// generate hash vals
	uint32_t hashvals[32];
	for (int ii = 0; ii < nrbanks; ++ii)
		hashvals[ii] = rand() & HASHMASK;

	int* seen = calloc(1 << NRHASHBITS, sizeof(int));

	int r = 0;
	for ( ; true; ++r) {
		/*
		// print banks
		for (int ii = 0; ii < nrbanks; ++ii)
			printf("%4d", b[ii]);
		printf("\n");
		*/
		int hashidx = gen_hash(b, nrbanks, hashvals);
		if (seen[hashidx]) {
			// printf("step %d == step %d\n", r, seen[hashidx] - 1);
			break;
		}
		seen[hashidx] = r + 1; // +1 since we start with r = 0

		redistribute(b, nrbanks);
	}
	printf("%d\n", r);
	return 0;
}
