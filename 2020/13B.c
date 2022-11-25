#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <inttypes.h>

// Chinese remainder: copied from 2016/15

// from: https://rosettacode.org/wiki/Modular_inverse#C
// returns x where (a * x) % m == 1
int64_t modinv(int64_t a, int64_t m) {
	int64_t b = m;
	int64_t x0 = 0;
	int64_t x1 = 1;
	int64_t t, q;
	if (b == 1)
		return 1;
	while (a > 1) {
		q = a / b;

		t = b;
		b = a % b;
		a = t;

		t = x0;
		x0 = x1 - q * x0;
		x1 = t;
	}
	if (x1 < 0)
		x1 += m;
	return x1;
}

struct mod_eqn { // x === a (mod m)
    int64_t a;
    int64_t m;
	struct mod_eqn* next; // allows LL of eqns
};

void print_eqn(struct mod_eqn* eq) {
	printf("x === %" PRId64 " (mod %" PRId64 ")\n", eq->a, eq->m);
}

void print_eqns(struct mod_eqn* eqns) {
	while (eqns) {
		print_eqn(eqns);
		eqns = eqns->next;
	}
}

// ALGO: see: https://www.interdb.jp/blog/pgsql/modmul/
int64_t modmul64(int64_t x, int64_t y, int64_t m) {
	// x * y  (mod m);  x * y can overflow 64 bit

	/* Interleaved modular multiplication algorithm
	 *
	 *   D.N. Amanor, et al, "Efficient hardware architecture for
	 *    modular multiplication on FPGAs", in Field Programmable
	 *    Logic and Apllications, 2005. International Conference on,
	 *    Aug 2005, pp. 539-542.
	 *
	 * This algorithm is usually used in the field of digital circuit
	 * design.
	 *
	 * Input: X, Y, M; 0 <= X, Y <= M;
	 * Output: R = X *  Y mod M;
	 * bits: number of bits of Y
	 * Y[i]: i th bit of Y
	 *
	 * 1. R = 0;
	 * 2. for (i = bits - 1; i >= 0; i--) {
	 * 3. 	R = 2 * R;
	 * 4. 	if (Y[i] == 0x1)
	 * 5. 		R += X;
	 * 6. 	if (R >= M) R -= M;
	 * 7.	if (R >= M) R -= M;
	 *   }
	 * In Steps 3 and 5, overflow should be avoided.
	 * Steps 6 and 7 can be instead of a modular operation (R %= M).
	 */

	if (x == 0 || y == 0 || m == 1)
		return 0;

	int64_t r = 0;
	x %= m;
	x = x < 0 ? x + m : x;
	y %= m;
	y = y < 0 ? y + m : y;

	// To reduce the for loop in the algorithm below
	if (x < y) {
		int64_t tmp = x;
		x = y;
		y = tmp;
	}
	// now x >= y

	int bits = 63;
	for (int ii = bits - 1; ii >= 0; --ii) {
		if (r > 0x3fffffffffffffff)
			/* To avoid overflow, transform from (2 * r) to
			 * (2 * r) % m, and further transform to
			 * mathematically equivalent form shown below:
			 */
			r = m - ((m - r) << 1);
		else
			r <<= 1;
		if ((y >> ii) & 0x1) {
			if (r > 0x7fffffffffffffff - x)
			      /* To calculate (r + x) without overflow, transform to (r + x) % m,
	               * and transform to mathematically equivalent form (r + x - m).
			       */
			      r += x - m;
			else
			      r += x;
		}
		r %= m;
	}

	return r;
}

void combine_mod_eqns(struct mod_eqn* eq1, struct mod_eqn* eq2, struct mod_eqn* result) {
	// chinese remainder on 2 eqns, see https://brilliant.org/wiki/chinese-remainder-theorem/?quiz=chinese-remainder-theorem
	int64_t a = eq1->a;
	int64_t m = eq1->m;
	int64_t b = eq2->a;
	int64_t n = eq2->m;
	int64_t mi = modinv(m, n);

	int64_t mn = m * n;
	result->m = mn;
	// result->a = (m * mi * (b - a) + a) % mn;
	result->a = (modmul64(modmul64(m, mi, mn), b - a, mn) + a) % mn;
}

void solve_equations(struct mod_eqn* eqns, struct mod_eqn* result) { // eqns is LL
	result->a = eqns->a;
	result->m = eqns->m;
	while (eqns->next) {
		eqns = eqns->next;
		combine_mod_eqns(eqns, result, result);
	}
}

bool read_next_id(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line) && *line != 'x')
		++line;
	if (*line == '\0')
		return false;
	if (*line == 'x')
		++line;
	else {
		while (isdigit(*line)) {
			num = num * 10 + *line - '0';
			++line;
		}
	}
	*pLine = line;
	*x = num;
	return true;
}

struct mod_eqn* get_eqns_from_line(char* line) {
	int id = 0;
	int count = 0;
	struct mod_eqn* eqns = NULL;

	while (read_next_id(&line, &id)) {
		if (id) { // x === id - count (mod id)
			struct mod_eqn* eq = malloc(sizeof(struct mod_eqn));
			eq->next = eqns; // LL
			eqns = eq;
			int remainder = (id - count) % id;
			remainder = remainder < 0 ? remainder + id : remainder;
			eq->a = remainder;
			eq->m = id;
		}
		++count;
	}
	return eqns;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	// read time from first line
	getline(&line, &len, stdin);

	// read IDs from second line
	while (getline(&line, &len, stdin) != -1) { // allows multiple puzzle lines
		struct mod_eqn* eqns = get_eqns_from_line(line);
		print_eqns(eqns);

		struct mod_eqn result;
		solve_equations(eqns, &result);
		int64_t t = result.a;
		printf("--------\n");
		print_eqn(&result);
		printf("%" PRId64 "\n", t);

		// check equations
		struct mod_eqn* eqns_ref = get_eqns_from_line(line);
		while (eqns_ref) {
			struct mod_eqn* eq = eqns_ref;
			if (eq->a != t % eq->m) {
				print_eqn(eq);
				printf("   WRONG: t === %" PRId64 " (mod %" PRId64 ")\n", t % eq->m, eq->m);
			}
			eqns_ref = eqns_ref->next;
			free(eq);
		}

		// free
		while (eqns) {
			struct mod_eqn* eq = eqns;
			eqns = eqns->next;
			free(eq);
		}
	}

	free(line);
	return 0;
}
