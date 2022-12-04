#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

int char2int(char c) {
	return (c >= 'a' && c <= 'z') ? c - 'a' + 1 : c - 'A' + 27;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	int contents[2 * 26]; // idx: item; bit 0: first bag, bit 1: second bag etc.
	int bagnr = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (bagnr == 0)
			memset(contents, 0, 2*26 * sizeof(int));

		int sz = 0;
		while (isalpha(line[sz]))
			++sz;

		int ii;
		for (ii = 0; ii < sz; ++ii)
			contents[char2int(line[ii]) - 1] |= (1 << bagnr);

		++bagnr;
		if (bagnr == 3) {
			for (ii = 0; ii < 2*26; ++ii) {
				if (contents[ii] == 7) {
					sum += ii + 1;
					break;
				}
			}
			bagnr = 0;
		}
	}
	free(line);

	printf("%d\n", sum);

	return 0;
}
