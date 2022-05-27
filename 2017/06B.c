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

// double hashing allows small memory map
#define NRHASHBITS 16
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

	// generate hash vals (double hashing)
	uint32_t hashvals1[32];
	uint32_t hashvals2[32];
	for (int ii = 0; ii < nrbanks; ++ii) {
		hashvals1[ii] = rand() & HASHMASK;
		hashvals2[ii] = rand() & HASHMASK;
	}
	int* seen1 = calloc(1 << NRHASHBITS, sizeof(int));
	int* seen2 = calloc(1 << NRHASHBITS, sizeof(int));

	int r = 0;
	int cyclelen = 0;
	for ( ; true; ++r) {
		/*
		// print banks
		for (int ii = 0; ii < nrbanks; ++ii)
			printf("%4d", b[ii]);
		printf("\n");
		*/
		int hashidx1 = gen_hash(b, nrbanks, hashvals1);
		int hashidx2 = gen_hash(b, nrbanks, hashvals2);
		if (seen1[hashidx1] && seen1[hashidx1] == seen2[hashidx2]) {
			cyclelen = r - seen1[hashidx1] + 1;
			break;
		}
		seen1[hashidx1] = r + 1; // +1 since we start with r = 0
		seen2[hashidx2] = r + 1; // +1 since we start with r = 0

		redistribute(b, nrbanks);
	}
	printf("%d\n", cyclelen);

	free(seen1);
	free(seen2);
	return 0;
}
