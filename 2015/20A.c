#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Project Euler 095.c:

// when we have prime factorization of x, we can easily find sum of divisors:
// If x = p1^e1 * p2^e2 * ... * pn^en, we can divide x by any product
// d = p1^e1' * p2^e2' * ... * pn^en', with 0 <= ek' <= ek for all k <= n
// Summing all these options, we can derive their sum using geometric series formula:
// sum(d) = (p1^(e1+1)-1)/(p1-1) * (p2^(e2+1)-1)/(p2-1) * ... * (pn^(en+1)-1)/(pn-1)
// We then only have to subtract x from that sum (because div by x itself is not counted)

int sum_divs_helper(int* x, int p) {
	// p shall be known to divide *x
	// returns (p^(e+1) - 1)/(p - 1)
	// divides p out of x
	int pw = p; // power of p
	while (*x % p == 0) {
		*x /= p;
		pw *= p;
	}
	return (pw - 1)/(p - 1);
}

int sum_divs_primes(int x) {
	if (x < 3) return x - 1; // 1 --> 0, 2 --> 1
	int prod = 1;
	if (x % 2 == 0)
		prod *= sum_divs_helper(&x, 2);
	if (x % 3 == 0)
		prod *= sum_divs_helper(&x, 3);
	int sqrtx = (int)sqrt(x);
	for (int sixk = 6; sixk - 1 <= sqrtx; sixk += 6) {
		if (x % (sixk - 1) == 0)
			prod *= sum_divs_helper(&x, (sixk - 1));
		if (x % (sixk + 1) == 0)
			prod *= sum_divs_helper(&x, (sixk + 1));
	}
	if (x > 1) // x itself is last prime factor with exp 1
		prod *= (x + 1);
	return prod;
}

int main(int argc, char* argv[]) {
	int target = 34000000;
	target /= 10;
	// lower limit: if all n would have divisors 1..n, their sum would be
	// n*(n+1)/2 >= target; lower limit we choose: sqrt(2*target)
	int x = sqrt(2*target);
	int sd = sum_divs_primes(x);
	while (sd < target) {
		++x;
		sd = sum_divs_primes(x);
	}
	printf("%d\n", x);
	return 0;
}
