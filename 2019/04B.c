#include <stdio.h>
#include <stdbool.h>

#define DIGITS 6

#define PW_LO   136818
#define PW_HIGH 685979

int inc_pw(int pw) {
	int d[DIGITS];
	int incidx = -1;
	for (int ii = DIGITS - 1; ii >= 0; --ii) {
		d[ii] = pw % 10;
		pw /= 10;
		// find first digit from right that is < 9
		if (d[ii] < 9 && incidx == -1)
			incidx = ii;
	}
	if (incidx == -1)
		return -1;
	++d[incidx];
	for (int ii = incidx + 1; ii < DIGITS; ++ii)
		d[ii] = d[ii - 1];
	// conv back to nr
	pw = 0;
	for (int ii = 0; ii < DIGITS; ++ii)
		pw = 10 * pw + d[ii];
	return pw;
}

int make_initial_pw(int pw) {
	int d[DIGITS];
	for (int ii = DIGITS - 1; ii >= 0; --ii) {
		d[ii] = pw % 10;
		pw /= 10;
	}
	int erridx = 1;
	for (; erridx < DIGITS; ++erridx)
		if (d[erridx] < d[erridx - 1])
			break;
	for (; erridx < DIGITS; ++erridx)
		d[erridx] = d[erridx - 1];
	// conv back to nr
	pw = 0;
	for (int ii = 0; ii < DIGITS; ++ii)
		pw = 10 * pw + d[ii];
	return pw;
}

bool has_double_dig(int pw) {
	int prevdig = pw % 10;
	pw /= 10;
	int count = 1;
	while (pw) {
		int d = pw % 10;
		if (d == prevdig)
			++count;
		else if (count == 2)
			return true;
		else
			count = 1;
		prevdig = d;
		pw /= 10;
	}
	return (count == 2);
}

int main(void) {
	int pw = PW_LO;
	pw = make_initial_pw(pw);
	int count = 0;
	while (pw <= PW_HIGH) {
		if (has_double_dig(pw))
			++count;
		pw = inc_pw(pw);
	}
	printf("%d\n", count);

	return 0;
}
