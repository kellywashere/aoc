#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dragonbit(char* input, int inputlen, int idx) {
	// base case 1
	if (idx < inputlen)
		return input[idx] == '1' ? 1 : 0;
	// check if idx points exactly to inserted 0
	int idx0 = inputlen;
	while (idx0 < idx)
		idx0 = 2*idx0 + 1;
	// base case 2
	if (idx0 == idx)
		return 0;
	// recurse
	// last inserted zero location
	idx = idx0 - idx - 1;
	return 1 - dragonbit(input, inputlen, idx);
}

int checksumbit(char* input, int disksize, int checksumsize, int idx) {
	// base case
	if (checksumsize >= disksize)
		return dragonbit(input, strlen(input), idx);
	int b0 = checksumbit(input, disksize, 2*checksumsize, 2*idx);
	int b1 = checksumbit(input, disksize, 2*checksumsize, 2*idx + 1);
	return (b0 == b1 ? 1 : 0);
}

int main(int argc, char* argv[]) {
	int disksize = 35651584;
	char* input = "10010000000110000";
	if (argc == 3) {
		input = argv[1];
		disksize = atoi(argv[2]);
	}

	int checksumsize = disksize;
	while (checksumsize % 2 == 0)
		checksumsize /= 2;

	for (int ii = 0; ii < checksumsize; ++ii)
		printf("%c", checksumbit(input, disksize, checksumsize, ii) ? '1' : '0');
	printf("\n");
	return 0;
}
