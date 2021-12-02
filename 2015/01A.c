#include <stdio.h>

int main(int argc, char* argv[]) {
	int c;
	int floor = 0;
	while ( (c = fgetc(stdin)) != EOF) {
		if (c == '(')
			++floor;
		else if (c == ')')
			--floor;
	}
	printf("%d\n", floor);
	return 1;
}
