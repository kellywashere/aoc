#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_triangle(int a, int b, int c) {
	return (a + b > c) && (a + c > b) && (b + c > a);
}

int main(int argc, char* argv[]) {
	int count = 0;
	int a[9];
	int idx = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!sscanf(line, "%d %d %d", &a[idx], &a[idx + 1], &a[idx + 2]))
			break;
		idx += 3;
		if (idx == 9) {
			for (int col = 0; col < 3; ++col)
				count += is_triangle(a[col], a[col + 3], a[col + 6]) ? 1 : 0;
			idx = 0;
		}
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
