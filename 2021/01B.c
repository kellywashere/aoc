#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int x1, x2, x3, x4;
	int count = 0;
	scanf("%d\n", &x1);
	scanf("%d\n", &x2);
	scanf("%d\n", &x3);
	while (scanf("%d\n", &x4) != EOF) {
		if (x4 > x1)
			++count;
		x1 = x2;
		x2 = x3;
		x3 = x4;
	}
	printf("%d\n", count);
	return 0;
}
