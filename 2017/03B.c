#include <stdio.h>
#include <stdlib.h>

void idx_to_xy(int idx, int* xpos, int* ypos) {
	// s parameter (03A) == idx + 1
	int s = idx + 1; // at idx 0 we have problem number 1, idx 1 has nr 2 etc
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

int xy_to_idx(int x, int y) {
	int s; // s =def= idx + 1
	// what number s appears at x,y.
	int absx = x < 0 ? -x : x;
	int absy = y < 0 ? -y : y;
	int maxxy = absx > absy ? absx : absy; // max of |x|, |y|
	int r = maxxy * 2 + 1;
	if (y == -maxxy) // bottom side
		s = x + r*r - maxxy;
	else if (x == -maxxy) // left side
		s = -maxxy + r*r - r + 1 - y;
	else if (y == maxxy) // top side
		s = maxxy + r*r - 3*r + 3 - x;
	else // right side
		s = y + maxxy + r*r - 4*r + 4;
	return s - 1; // idx = s - 1
}

int dx[] = {-1, -1, -1,  0, 0,  1, 1, 1};
int dy[] = {-1,  0,  1, -1, 1, -1, 0, 1};

int main(int argc, char* argv[]) {
	int input = 277678; // number of the square (my input)
	if (argc > 1)
		input = atoi(argv[1]);

	int values[1024]; // TODO: dynamic array...
	values[0] = 1; // start value
	int answer = 0;
	for (int idx = 1; !answer && idx < 1024; ++idx) {
		int x,y;
		idx_to_xy(idx, &x, &y);
		values[idx] = 0;
		for (int ii = 0; ii < 8; ++ii) {
			int idx2 = xy_to_idx(x + dx[ii], y + dy[ii]);
			values[idx] += idx2 < idx ? values[idx2] : 0;
		}
		if (values[idx] > input)
			answer = values[idx];
	}
	printf("%d\n", answer);

	return 0;
}
