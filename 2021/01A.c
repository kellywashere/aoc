#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int x, xprev;
	int count = 0;
	scanf("%d\n", &xprev);
	while (scanf("%d\n", &x) != EOF) {
		if (x > xprev)
			++count;
		xprev = x;
	}
	printf("%d\n", count);
	return 0;
}
