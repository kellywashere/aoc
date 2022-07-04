#include <stdio.h>

int calc_fuel(int mass) {
	int f = mass / 3 - 2;
	int tot = 0;
	while (f > 0) {
		tot += f;
		f = f / 3 - 2;
	}
	return tot;
}


int main(void) {
	int mass;
	int tot = 0;

	while (scanf("%d", &mass) == 1)
		tot += calc_fuel(mass);
	
	printf("%d\n", tot);

	return 0;
}
