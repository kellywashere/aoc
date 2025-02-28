#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

bool read_int_nowhiteskip(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

bool find_mul(char** pLine, int* prod) {
	char* mul_loc = strstr(*pLine, "mul(");
	if (mul_loc == NULL) { // no more mul, we can set *pLine to end of str
		while (**pLine != '\0')
			++(*pLine);
		return false;
	}
	*pLine = mul_loc + 4;
	int n1, n2;
	if (!read_int_nowhiteskip(pLine, &n1))
		return false;
	if (**pLine != ',')
		return false;
	++(*pLine); // skip ,
	if (!read_int_nowhiteskip(pLine, &n2))
		return false;
	if (**pLine != ')')
		return false;
	*prod = n1 * n2;
	return true;
}

bool remove_dont(char* line, bool mul_enbld) {
	// when mul_enbld, mul is enabled at start if this line
	// returns false when last keyword found was "dont"
	bool ends_enabled = true;
	char* dont_loc = line;
	if (mul_enbld)
		dont_loc = strstr(line, "don't()");

	while (dont_loc) {
		char* do_loc = strstr(dont_loc + 7, "do()");
		if (do_loc == NULL) {
			ends_enabled = false;
			do_loc = line + strlen(line);
		}
		for (char* p = dont_loc; p != do_loc; ++p)
			*p = '-';
		line = do_loc;
		dont_loc = strstr(line, "don't()");
	}

	return ends_enabled;
}

int main(int argc, char* argv[]) {
	uint64_t tot = 0;

	char *line = NULL;
	size_t len = 0;
	bool enabled = true;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;

		enabled = remove_dont(l, enabled);

		int prod = 0;
		while (*l != '\0') {
			if (find_mul(&l, &prod))
				tot += prod;
		}
	}

	printf("%" PRId64 "\n", tot);
	free(line);
	return 0;
}
