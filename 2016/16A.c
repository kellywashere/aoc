#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* dragon_step(char* instr) {
	// re-allocs the instr!
	int inlen = strlen(instr);
	int outlen = 2*inlen + 1;
	char* outstr = realloc(instr, outlen + 1);
	for (int ii = 0; ii < inlen; ++ii)
		outstr[outlen - ii - 1] = instr[ii] == '0' ? '1' : '0';
	outstr[inlen] = '0';
	outstr[outlen] = '\0';
	printf("After dragon-step (len is %lu): %s\n", strlen(outstr), outstr);
	return outstr;
}

void checksum(char* inoutstr) { // in-place!
	int inlen = strlen(inoutstr);
	if (inlen % 2 != 0)
		fprintf(stderr, "checksum called on string with odd length: undefined!\n");
	int outlen = inlen / 2;
	for (int ii = 0; ii < outlen; ++ii)
		inoutstr[ii] = inoutstr[2*ii] == inoutstr[2*ii + 1] ? '1' : '0';
	inoutstr[outlen] = '\0';
	printf("After checksum (len is %lu): %s\n", strlen(inoutstr), inoutstr);
}

int main(int argc, char* argv[]) {
	int disksize = 272;
	char* input = "10010000000110000";
	if (argc == 3) {
		input = argv[1];
		disksize = atoi(argv[2]);
	}
	char* data = malloc((strlen(input) + 1) * sizeof(char));
	strcpy(data, input);

	while (strlen(data) < disksize)
		data = dragon_step(data);
	data[disksize] = '\0'; // cut off unneded part
	
	do {
		checksum(data);
	} while (strlen(data) % 2 == 0);

	free(data);
	return 0;
}
