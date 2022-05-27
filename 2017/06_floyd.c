// Floyd's tortoise and hare: https://en.wikipedia.org/wiki/Cycle_detection
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

void copy_state(int* dst, int* src, int nrbanks) {
	for (int ii = 0; ii < nrbanks; ++ii)
		dst[ii] = src[ii];
}

bool equal_state(int* a, int* b, int nrbanks) {
	for (int ii = 0; ii < nrbanks; ++ii)
		if (a[ii] != b[ii])
			return false;
	return true;
}

int main(int argc, char* argv[]) {
	int x;
	int b[32]; // max nr of banks foreseen: 32
	int t[32]; // tortoise
	int h[32]; // hare

	int nrbanks = 0; // banks used
	while (scanf("%d", &x) == 1)
		b[nrbanks++] = x;

	copy_state(t, b, nrbanks);
	redistribute(t, nrbanks); // tortoise = f(x0)
	copy_state(h, t, nrbanks);
	redistribute(h, nrbanks);  // hare = f(f(x0))

	while (!equal_state(t, h, nrbanks)) {
		redistribute(t, nrbanks); // tortoise := f(tortoise)
		redistribute(h, nrbanks);
		redistribute(h, nrbanks); // hare := f(f(hare))
	}

	int mu = 0;
	copy_state(t, b, nrbanks); // tortoise = x0
	while (!equal_state(t, h, nrbanks)) {
		redistribute(t, nrbanks); // tortoise := f(tortoise)
		redistribute(h, nrbanks); // hare := f(hare)
		++mu;
	}

	int lambda = 1;
	copy_state(h, t, nrbanks);
	redistribute(h, nrbanks); // hare = f(tortoise)
	while (!equal_state(t, h, nrbanks)) {
		redistribute(h, nrbanks); // hare := f(hare)
		++lambda;
	}
	printf("mu     = %d\n", mu);
	printf("lambda = %d\n", lambda);
	printf("Answer to 06A = mu + lambda = %d\n", mu + lambda);
	printf("Answer to 06B = lambda = %d\n", lambda);
	return 0;
}
