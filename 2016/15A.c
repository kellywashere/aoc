#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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

int modinv(int x, int m) {
	int gcdval, mcoef, xcoef;
	extended_gcd(m, x, &gcdval, &mcoef, &xcoef);
	if (gcdval != 1) {
		fprintf(stderr, "gcd(%d,%d) != 1\n", x, m);
		return 0;
	}
	int inv = xcoef % m;
	inv = inv < 0 ? inv + m : inv;
	return inv;
}

struct mod_eqn { // x === a (mod m)
    int a;
    int m;
	struct mod_eqn* next; // allows LL of eqns
};

void print_eqn(struct mod_eqn* eq) {
	printf("x === %d (mod %d)\n", eq->a, eq->m);
}

void print_eqns(struct mod_eqn* eqns) {
	while (eqns) {
		print_eqn(eqns);
		eqns = eqns->next;
	}
}

void combine_mod_eqns(struct mod_eqn* eq1, struct mod_eqn* eq2, struct mod_eqn* result) {
	// chinese remainder on 2 eqns, see https://brilliant.org/wiki/chinese-remainder-theorem/?quiz=chinese-remainder-theorem
	int a = eq1->a;
	int m = eq1->m;
	int b = eq2->a;
	int n = eq2->m;
	int mi = modinv(m, n);

	int mn = m * n;
	result->m = mn;
	result->a = (m * mi * (b - a) + a) % mn;
	result->a = result->a < 0 ? result->a + mn : result->a;
}

void solve_equations(struct mod_eqn* eqns, struct mod_eqn* result) { // eqns is LL
	result->a = eqns->a;
	result->m = eqns->m;
	while (eqns->next) {
		eqns = eqns->next;
		combine_mod_eqns(eqns, result, result);
	}
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && (*line < '0' || *line > '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

struct mod_eqn* get_mod_eqn_from_line(char* line) {
	struct mod_eqn* eq = malloc(sizeof(struct mod_eqn));
	eq->next = NULL;
	int disc_nr = read_int(&line);
	int nr_positions = read_int(&line);
	read_int(&line); // time=0: ignored
	int init_pos = read_int(&line);
	printf("Disc #%d: m = %d, p0 = %d\n", disc_nr, nr_positions, init_pos);
	eq->a = -(disc_nr + init_pos);
	eq->m = nr_positions;
	return eq;
}

int main(int argc, char* argv[]) {
	struct mod_eqn* eqns = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		struct mod_eqn* eqn = get_mod_eqn_from_line(line);
		eqn->next = eqns;
		eqns = eqn;
	}
	print_eqns(eqns);
	struct mod_eqn result;
	solve_equations(eqns, &result);
	printf("--------\n");
	print_eqn(&result);
	free(line);
	while (eqns) {
		struct mod_eqn* eq = eqns;
		eqns = eqns->next;
		free(eq);
	}
	return 0;
}
