#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void reverse_list(int* list, int pos, int len) {
	for (int ii = 0; ii < len / 2; ++ii) {
		int idx1 = (pos + ii) & 255;
		int idx2 = (pos + len - ii - 1) & 255;
		int v = list[idx1];
		list[idx1] = list[idx2];
		list[idx2] = v;
	}
}

int suffix[] = {17, 31, 73, 47, 23};

void knot_hash(const char* str, int* hash) {
	int sparse[256];
	char input[1024];
	// init sparse hash data
	for (int ii = 0; ii < 256; ++ii)
		sparse[ii] = ii;
	int pos = 0;
	int skipsize = 0;
	strncpy(input, str, 1023);
	// add 17, 31, 73, 47, 23
	int inlen = strlen(input);
	for (int ii = 0; ii < 5; ++ii)
		input[inlen++] = suffix[ii];
	input[inlen] = '\0';
	for (int round = 0; round < 64; ++round) {
		for (int ii = 0; input[ii] != '\0'; ++ii) {
			reverse_list(sparse, pos, input[ii]);
			pos = (pos + input[ii] + skipsize) & 255;
			skipsize = (skipsize + 1) & 255;
		}
	}
	// sparse to dense:
	for (int didx = 0; didx < 16; ++didx) {
		hash[didx] = 0;
		for (int ii = 0; ii < 16; ++ii)
			hash[didx] ^= sparse[16 * didx + ii];
	}
}

int popcount(int x) {
	int count = 0;
	for ( ; x > 0; x >>= 1)
		count += x & 1;
	return count;
}

int main(int argc, char* argv[]) {
	char* input = "jzgqcdpd";
	if (argc > 1)
		input = argv[1];

	int count_used = 0;
	for (int row = 0; row < 128; ++row) {
		char buf[80];
		int dense[16];
		sprintf(buf, "%s-%d", input, row);

		knot_hash(buf, dense);
		for (int ii = 0; ii < 16; ++ii)
			count_used += popcount(dense[ii]);
	}
	printf("%d\n", count_used);

	return 0;
}
