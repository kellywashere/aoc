#include <stdio.h>
#include <inttypes.h>

uint64_t f(uint64_t r1) {
	uint64_t r4 = r1 | 0x10000;
	r1 = 0x39F737;
	do {
		r1 += (r4 & 0xFF);
		r1 &= 0xFFFFFF;
		r1 *= 65899;
		r1 &= 0xFFFFFF;

		r4 >>= 8;
	} while (r4);
	return r1;
}

uint64_t repf(uint64_t r1, int n) {
	while (n--)
		r1 = f(r1);
	return r1;
}

void floyd(uint64_t x0, int* mu, int* lambda) {
	// https://en.wikipedia.org/wiki/Cycle_detection
	uint64_t tortoise = f(x0);
	uint64_t hare = f(tortoise);

	while (tortoise != hare) {
		tortoise = f(tortoise);
		hare = f(f(hare));
	}

	*mu = 0;
	tortoise = x0;
	while (tortoise != hare) {
		tortoise = f(tortoise);
		hare = f(hare);
		++(*mu);
	}

	*lambda = 1;
	hare = f(tortoise);
	while (tortoise != hare) {
		hare = f(hare);
		++(*lambda);
	}
}

int main(void) {
	int mu = 0;
	int lambda = 0;
	floyd(0, &mu, &lambda);
	// last value before repeating: running f mu + lambda - 1 times
	printf("%" PRIu64 "\n", repf(0, mu + lambda - 1));


	return 0;
}
