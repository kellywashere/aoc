#include <stdio.h>
#include <stdlib.h>

void extended_gcd(int a, int b, int* gcdout, int* acoef, int* bcoef) {
// See: https://brilliant.org/wiki/extended-euclidean-algorithm/
	int s = 0;
	int old_s = 1;
	int t = 1;
	int old_t = 0;
	int r = b;
	int old_r = a;
	while (r) {
		int q = old_r / r;
		
		int newval = old_r - q * r;
		old_r = r;
		r = newval;

		newval = old_s - q * s;
		old_s = s;
		s = newval;

		newval = old_t - q * t;
		old_t = t;
		t = newval;
	}
	*gcdout = old_r;
	*acoef = old_s;
	*bcoef = old_t;
}

void usage(char* progname) {
	printf("Usage: %s y m\n", progname);
	printf("Calculates modular inverse of y (mod m)\n");
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	int y = atoi(argv[1]);
	int m = atoi(argv[2]);
	int gcdval, mcoef, ycoef;

	extended_gcd(m, y, &gcdval, &mcoef, &ycoef);
	if (gcdval != 1) {
		printf("gcd(y,n) should equal 1 for this to work\n");
		return 1;
	}
	int inv = ycoef % m;
	inv = inv < 0 ? inv + m : inv;
	printf("Modular inverse of %d (mod %d) = %d\n", y, m, inv);
	// printf("%d * %d = %d = %d (mod %d)\n", inv, y, inv * y, (inv * y) % m, m);

	return 0;
}
