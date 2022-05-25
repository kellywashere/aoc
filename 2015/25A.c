// see notes25.txt
#include <stdio.h>
#include <inttypes.h>

int read_int_from_stdin() {
	char c;
	int x = 0;
	c = getc(stdin);
	while (c != EOF && (c < '0' || c > '9'))
		c = getc(stdin);
	while (c >= '0' && c <= '9') {
		x = 10 * x + c - '0';
		c = getc(stdin);
	}
	return x;
}

// n(r,c) = (r+c-1)*(r+c-2)/2 + c

int main(int argc, char* argv[]) {
	int r = read_int_from_stdin();
	int c = read_int_from_stdin();

	int mult = 252533;   // is prime
	int mod  = 33554393; // is prime
	
	int n = (r+c-1)*(r+c-2)/2 + c; // we need n-th code

	uint64_t code = 20151125;
	int steps = n - 1;
	for (int ii = 0; ii < steps; ++ii) {
		code = (code * mult) % mod;
	}
	printf("%" PRIu64 "\n", code);

	return 0;
}
