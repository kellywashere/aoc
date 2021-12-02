#include <stdio.h>

int main(int argc, char* argv[]) {
	int c;
	int floor = 0;
	int count = 0;
	while (floor >= 0 && (c = fgetc(stdin)) != EOF) {
		++count;
		if (c == '(')
			++floor;
		else if (c == ')')
			--floor;
	}
	printf("%d\n", count);
	return 1;
}
