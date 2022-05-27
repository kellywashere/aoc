#include <stdio.h>
#include <stdlib.h>

void calc_xy_pos(int s, int* xpos, int* ypos) {
	// which ring is this nr on?
	int r; // ring side length (always odd)
	// find odd r s.t. (r-2)^2 < s <= r^2
	for (r = 1; r*r < s; r += 2)
		;
	// nr in bottom-right corner of ring with width r = r^2
	int smax = r*r;
	int x = (r - 1)/2; // x loc of smax
	int y = -x;
	if (s > smax - r) // nr we are looking for in bottom part of ring
		x -= smax - s;
	else if (s > smax - 2*r + 1) { // left side
		x = -x;
		y += (smax - r + 1) - s;
	}
	else if (s > smax - 3*r + 2) { // top side
		y = -y;
		x -= s - (smax - 3*r + 3);
	}
	else // right side
		y += s - (smax - 4*r + 4);
	*xpos = x;
	*ypos = y;
}

int main(int argc, char* argv[]) {
	int s = 277678; // number of the square (my input)
	if (argc > 1)
		s = atoi(argv[1]);

	int x,y;
	calc_xy_pos(s, &x, &y);
	int dist = (x < 0 ? -x : x) + (y < 0 ? -y : y);
	printf("%d\n", dist);
	return 0;
}
