#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define PWLEN 8

void inc_password(char* pw) {
	// in place
	int idx = PWLEN - 1;
	while (pw[idx] == 'z' && idx >= 0)
		--idx;
	if (idx >= 0) {
		++pw[idx];
		if (pw[idx] == 'i' || pw[idx] == 'l' || pw[idx] == 'o')
			++pw[idx];
	}
	while (++idx < PWLEN)
		pw[idx] = 'a';
}

bool check_pw(const char* pw) {
	bool pw_okay = false;
	int ii, jj;
	// check increasing seq
	for (ii = 0; !pw_okay && ii < PWLEN - 2; ++ii)
		pw_okay = pw[ii + 1] == pw[ii] + 1  &&  pw[ii + 2] == pw[ii + 1] + 1;
	if (!pw_okay)
		return false;
	// 2nd req is taken care of by inc_pw
	// check double distinct pair
	pw_okay = false;
	for (ii = 0; !pw_okay && ii < PWLEN - 3; ++ii) {
		if (pw[ii] == pw[ii + 1]) {
			for (jj = ii + 2; !pw_okay && jj < PWLEN - 1; ++jj)
				pw_okay = pw[jj] == pw[jj + 1];
		}
	}
	return pw_okay;
}

int main(int argc, char* argv[]) {
	char pw[] = "hepxcrrq";
	if (argc > 1) {
		if (strlen(argv[1]) != PWLEN)
			fprintf(stderr, "arg should have len %d\n", PWLEN);
		else
			strcpy(pw, argv[1]);
	}
	do {
		inc_password(pw);
	} while (!check_pw(pw));
	// and again
	do {
		inc_password(pw);
	} while (!check_pw(pw));
	printf("%s\n", pw);

	return 0;
}
