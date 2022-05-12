#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_triangle(int a, int b, int c) {
	return (a + b > c) && (a + c > b) && (b + c > a);
}

int main(int argc, char* argv[]) {
	int count = 0;
	int a, b, c;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (sscanf(line, "%d %d %d", &a, &b, &c))
			count += is_triangle(a, b, c) ? 1 : 0;
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
