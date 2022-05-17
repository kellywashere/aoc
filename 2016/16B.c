// see notes16.txt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int suma(char* a, int x) {
	// sum mod 2 of first x bits of a
	int sum = 0;
	for (int ii = 0; ii < x; ++ii)
		sum += a[ii] == '1' ? 1 : 0;
	return sum % 2;
}

int sumb(char* a, int x) {
	// sum mod 2 of first x bits of b = rev(~a)
	int La = strlen(a);
	int sum = 0;
	for (int ii = 0; ii < x; ++ii)
		sum += a[La - ii - 1] == '1' ? 0 : 1;
	return sum % 2;
}

int sum_fillers(int x) {
	// sum (mod 2) of first x filler bits
	if (x < 3)
		return 0;
	// find max number of dragon steps s s.t. L(s) <= x
	int s = 0;
	int Ls = 1;
	while (2*Ls + 1 <= x) {
		++s;
		Ls = 2*Ls + 1;
	}
	if (Ls == x) // x contains exactly a whole pattern: sum = 1 (mod 2)
		return 1;
	// S(F(s+1),X) = S(F(s)) + 0 + S(rev(~F(s), X - L(s) - 1)
	int m = x - Ls - 1; // bits remaining after filler 0
	// S(rev(~F(s), m) = S(~F(s)) - S(~F(s), L(s) - m) = S(~F(s), L(s) - m)
	// S(~F(s), L(s) - m) = 1 + m + s(F(s), L(s) - m)  (mod 2)
	// So S(F(s+1),X) = 1 + 0 + (1 + m + s(F(s), L(s) - m)) = m + s(F(s), L(s) - m)
	return (m + sum_fillers(Ls - m)) % 2;
}

int sum_pattern(char* a, int x) {
	// sum of first x bits of generated dragon pattern
	int La = strlen(a);
	int seclen = 2*La + 2; // length of a f(n) b f(n+1)
	int k = x / seclen;
	int m = x - k*seclen; // remaining bits after sections
	// k * ab --> sum is k*La
	int sum = (k*La) % 2; // mod 2 sum is enough
	int nrfillers = 2*k;
	if (m <= La)
		sum += suma(a, m);
	else { // full a pattern
		sum += suma(a, La);
		if (m > La)
			++nrfillers;
		if (m > La + 1) // some bits of b
			sum += sumb(a, m - La - 1);
	}
	sum += sum_fillers(nrfillers);
	return sum % 2;
}

int main(int argc, char* argv[]) {
	int disksize = 35651584;
	char* input = "10010000000110000";
	if (argc == 3) {
		input = argv[1];
		disksize = atoi(argv[2]);
	}

	int checksumsize = disksize;
	int blocksize = 1;
	while (checksumsize % 2 == 0) {
		checksumsize /= 2;
		blocksize *= 2;
	}

	// check sum
	int prev_sum = 0;
	for (int ii = 1; ii <= checksumsize; ++ii) {
		int sum = sum_pattern(input, ii * blocksize);
		int sum_block = (sum + prev_sum) % 2; // mod 2: + is same as -, + avoids neg numbers
		int paritybit = (1 + sum_block) % 2;
		printf("%c", paritybit ? '1' : '0');
		prev_sum = sum;
	}
	printf("\n");



	/* print filler via sum, to verify
	int prev_sum = 0;
	for (int ii = 1; ii < 20; ++ii) {
		int new_sum = sum_fillers(ii);
		printf("%c", new_sum == prev_sum ? '0' : '1');
		prev_sum = new_sum;
	}
	printf("\n");
	*/

	return 0;
}
