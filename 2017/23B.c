#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

bool is_prime(int x) {
	if (x < 2)
		return false;
	if ((x == 2) || (x == 3))
		return true;
	if ((x % 2 == 0) || (x % 3 == 0))
		return false;
	int sqrtx = (int)sqrt(x);
	for (int sixk = 6; sixk - 1 <= sqrtx; sixk += 6) {
		if ((x % (sixk - 1) == 0) || (x % (sixk + 1) == 0))
			return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	// program checks if nr is prime
	// below the functional representation:
	int b = 108400;
	int c = b + 17000;
	int h = 0;
	for (int b = 108400; b <= c; b += 17) {
		if (!is_prime(b))
			++h;
	}
	printf("%d\n", h);

	return 0;
}
