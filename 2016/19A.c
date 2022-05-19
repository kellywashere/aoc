#include <stdio.h>
#include <stdlib.h>

//see notes19.txt

int main(int argc, char* argv[]) {
	int a = 1; // lowest valued player
	int s = 1; // step between nr of two neighboring players
	int b = 3014603; // puzzle input
	int t = 1; // player with turn
	if (argc > 1)
		b = atoi(argv[1]);
	while (b > a) {
		a = (t == a) ? a : a + s;
		s = 2 * s;
		int bnext = a + ((b - a) / s) * s;
		t = bnext < b ? a : bnext;
		b = bnext;
	}
	printf("%d\n", b);
	return 0;
}
