#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
	int c = getc(stdin);
	int firstc = c;
	bool done = false;
	int sum = 0;
	while (!done) {
		int prevc = c;
		c = getc(stdin);
		if (c >= '0' && c <= '9') {
			if (c == prevc)
				sum += c - '0';
		}
		else {
			if (prevc == firstc)
				sum += prevc - '0';
			done = true;
		}
	}
	printf("%d\n", sum);
	return 0;
}
