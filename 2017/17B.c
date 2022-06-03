#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int len = 1;
	int pos = 0;
	int valat1 = 0;

	int maxnr = 50000000;
	int step = 303;
	for ( ; len <= maxnr; ++len) {
		// make n steps without wrapping
		int n = (len - pos - 1)/step;
		// in these n steps we insert len, len+1, .., len+n-1
		if (len + n - 1 > maxnr)
			n = maxnr - len;
		pos += n*(step + 1);
		len += n;
		// now the wrapping step
		pos = (pos + step) % len + 1;
		if (pos == 1)
			valat1 = len;
	}
	printf("%d\n", valat1);
	return 0;
}
