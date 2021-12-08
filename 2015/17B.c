#include <stdio.h>
#include <stdlib.h>

#define MAX_CONTAINERS 30

int count_combinations(int cont[], int len, int tot, int nr_cont_remaining) {
	if (tot < 0 || nr_cont_remaining < 0)
		return 0;
	if (tot == 0)
		return (nr_cont_remaining == 0 ? 1 : 0);
	// tot > 0 here
	if (len == 0 || nr_cont_remaining == 0)
		return 0;
	return (count_combinations(cont + 1, len - 1, tot - cont[0], nr_cont_remaining - 1) +
			count_combinations(cont + 1, len - 1, tot, nr_cont_remaining));
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
	int nr_cont = 1;
	int nr_combs = 0;
	while (!nr_combs && nr_cont <= len) {
		nr_combs = count_combinations(containers, len, liters, nr_cont);
		if (!nr_combs)
			++nr_cont;
	}
	printf("%d\n", nr_combs);

	return 0;
}
