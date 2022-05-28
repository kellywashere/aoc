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

void remove_leading_space(char* l) {
	int idx1 = 0;
	int idx2 = 0;
	while (l[idx2] && isspace(l[idx2]))
		++idx2;
	while (l[idx2])
		l[idx1++] = l[idx2++];
	l[idx1] = '\0';
}

void remove_trailing_space(char* l) {
	int idx = 0;
	while (l[idx])
		++idx;
	while (idx > 0 && isspace(l[idx - 1])) {
		--idx;
		l[idx] = '\0';
	}
}

void hash_round(int* sparse, int* pos, int* skipsize, char* input) {
	for ( ; *input; ++input) {
		reverse_list(sparse, *pos, *input);
		*pos = (*pos + *input + *skipsize) & 255;
		*skipsize = (*skipsize + 1) & 255;
	}
}

void sparse_to_dense(int* sparse, int* dense) {
	for (int didx = 0; didx < 16; ++didx) {
		dense[didx] = 0;
		for (int ii = 0; ii < 16; ++ii)
			dense[didx] ^= sparse[16 * didx + ii];
	}
}

void dense_to_str(int* dense, char* densestr) {
	for (int ii = 0; ii < 16; ++ii)
		sprintf(densestr + 2*ii, "%02x", dense[ii]);
}

int suffix[] = {17, 31, 73, 47, 23};

int main(int argc, char* argv[]) {
	int sparse[256];
	int dense[16];
	// init sparse hash data
	for (int ii = 0; ii < 256; ++ii)
		sparse[ii] = ii;
	int pos = 0;
	int skipsize = 0;

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin); // reads input line

	remove_leading_space(line);
	remove_trailing_space(line);
	// add 17, 31, 73, 47, 23
	int inlen = strlen(line);
	for (int ii = 0; ii < 5; ++ii)
		line[inlen++] = suffix[ii];
	line[inlen] = '\0';

	for (int ii = 0; ii < 64; ++ii)
		hash_round(sparse, &pos, &skipsize, line);
	sparse_to_dense(sparse, dense);

	char densestr[32 + 1];
	dense_to_str(dense, densestr);
	printf("%s\n", densestr);

	free(line);
	return 0;
}
