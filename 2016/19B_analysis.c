// Bad algo, used to analyze and maybe find patterns
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
	int n = 3014603; // puzzle input
	if (argc > 1)
		n = atoi(argv[1]);
	bool* alive = malloc(n * sizeof(bool));
	for (int ii = 0; ii < n; ++ii)
		alive[ii] = true;
	int p = n / 2; // player to eliminate
	int nremain = n;
	do {
		alive[p] = false;
		if (n < 100) printf("Player %d eliminated\n", p + 1);
		--nremain;
		int dp = (nremain % 2 == 1) ? 1 : 2; // next player to eliminate: dp players further than p
		for (int ii = 0; ii < dp; ++ii) {
			do {
				p = (p + 1) % n;
			} while (!alive[p]);
		}
	} while (nremain > 1);
	int t = 0;
	while (!alive[t])
		++t;
	printf("Player %d wins\n", t + 1);
	return 0;
}

