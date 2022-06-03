#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int buf[2018];
	buf[0] = 0;
	int len = 1;
	int pos = 0;

	int maxnr = 2017;
	if (argc > 1)
		maxnr = atoi(argv[1]);

	int step = 303;
	if (argc > 2)
		step = atoi(argv[2]);

	for ( ; len <= maxnr; ++len) {
		pos = (pos + step) % len + 1;
		for (int ii = len; ii > pos; --ii)
			buf[ii] = buf[ii - 1];
		buf[pos] = len;
	}
	printf("%d\n", buf[pos + 1]);
	return 0;
}
