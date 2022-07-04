#include <stdio.h>

int main(void) {
	int mass;
	int tot = 0;

	while (scanf("%d", &mass) == 1)
		tot += mass / 3 - 2;
	
	printf("%d\n", tot);

	return 0;
}
