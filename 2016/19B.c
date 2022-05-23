// Bad algo, used to analyze and maybe find patterns
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
	int n = 3014603; // puzzle input
	if (argc > 1)
		n = atoi(argv[1]);
	// k s.t. 3^k <= n < 3*3^k
	int pw3 = 1; // pw3 = 3^k
	while (3 * pw3 <= n)
		pw3 *= 3;
	int w;
	if (n < 2*pw3 + 1)
		w = n - pw3;
	else
		w = 2*n - 3*pw3;
	printf("Player %d wins\n", w);
	return 0;
}

