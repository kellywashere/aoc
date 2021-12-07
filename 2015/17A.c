#include <stdio.h>
#include <stdlib.h>

#define MAX_CONTAINERS 30

int count_combinations(int cont[], int len, int tot) {
	if (tot < 0)
		return 0;
	if (tot == 0)
		return 1;
	if (len == 0)
		return 0;
	return (count_combinations(cont + 1, len - 1, tot - cont[0]) +
			count_combinations(cont + 1, len - 1, tot));
}

int main(int argc, char* argv[]) {
	int liters = 150;
	if (argc > 1)
		liters = atoi(argv[1]);
	int x;
	int containers[MAX_CONTAINERS];
	int len = 0;
	while (scanf("%d", &x) != EOF)
		containers[len++] = x;
	printf("%d\n", count_combinations(containers, len, liters));

	return 0;
}
