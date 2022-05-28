#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
/* Coordinate system:

  /    \    /
 + 0,4  +--+ 2,4
  \    /    \
   +--+ 1,3  +--
  /    \    /
-+ 0,2  +--+ 2,2
  \    /    \
   +--+ 1,1  +--
  /    \    /
 + 0,0  +--+ 2,0
  \    /    \
*/

bool read_dir(char** pLine, int* dx, int* dy) {
	char* l = *pLine;
	while (*l && *l != 'n' && *l != 's')
		++l;
	if (*l == '\0')
		return false;
	*dy = *l == 'n' ? 1 : -1; // can still double
	++l;
	if (*l == 'e') {
		*dx = 1;
		++l;
	}
	else if (*l == 'w') {
		*dx = -1;
		++l;
	}
	else {
		*dx = 0;
		*dy *= 2;
	}
	*pLine = l;
	return true;
}

int distance_to_origin(int x, int y) {
	x = x < 0 ? -x : x;
	y = y < 0 ? -y : y;
	if (y > x) // then go x steps sw, ending in (0, y-x), then go (y-x)/2 steps s
		return x + (y-x)/2;
	// x >= y: each step: move either se or ne (x reduces by 1), choosing between the two based on sign of y
	return x;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	char* l = line;

	int x = 0;
	int y = 0;
	int dx = 0;
	int dy = 0;
	int maxdist = 0;
	while (read_dir(&l, &dx, &dy)) {
		x += dx;
		y += dy;
		int d = distance_to_origin(x, y);
		maxdist = d > maxdist ? d : maxdist;
	}
	printf("%d\n", maxdist);

	free(line);
	return 0;
}
