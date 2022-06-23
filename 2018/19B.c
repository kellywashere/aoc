#include <stdio.h>
#include <math.h>

// See notes19.txt
// Program finds sum of divisors of r2

int main(void) {

	int r0 = 1;
	int r2 = 905;
	if (r0)
		r2 = 10551305;

	// calc sum of divisors of r2
	r0 = 0;
	int sqrt_r2 = sqrt(r2);
	for (int r1 = 1; r1 <= sqrt_r2; ++r1) {
		if (r2 % r1 == 0) {
			r0 += r1;
			r0 += r2 / r1;
		}
	}
	if (sqrt_r2 * sqrt_r2 == r2)
		r0 -= sqrt_r2; // was counted double

	printf("%d\n", r0);

	return 0;
}
