#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int sum_divs_limited(int x, int l) {
	// find sum of divisors d_n of x, s.t. l*d_n >= x
	// define k_n = x/d_n ==> d_n = x/k_n >= x/l
	// 1 <= k_n <= l
	int sd = x;
	for (int k = 2; k <= l; ++k)
		if (x % k == 0)
			sd += x/k;
	return sd;
}

int main(int argc, char* argv[]) {
	int target = 34000000;
	target = (target + 10)/11; // ceil(target/11)
	// lower limit: if all n would have divisors 1..n, their sum would be
	// n*(n+1)/2 >= target; lower limit we choose: sqrt(2*target)
	int x = sqrt(2*target); // x: house nr
	int sd = sum_divs_limited(x, 50);
	while (sd < target) {
		++x;
		sd = sum_divs_limited(x, 50);
	}
	printf("%d\n", x);
	return 0;
}
