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
	while (getline(&line, &len, stdin) != -1) {
		int sz = 0;
		while (isalpha(line[sz]))
			++sz;
		bool comp1[2 * 26] = {false};
		int ii;
		for (ii = 0; ii < sz / 2; ++ii)
			comp1[char2int(line[ii]) - 1] = true;
		for (; ii < sz; ++ii) {
			int x = char2int(line[ii]);
			if (comp1[x - 1]) {
				sum += x;
				break;
			}
		}
	}
	free(line);

	printf("%d\n", sum);

	return 0;
}
