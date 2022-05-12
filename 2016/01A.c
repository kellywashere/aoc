#include <stdio.h>

int main(int argc, char* argv[]) {
	char c;
	int dir; // 1: left, -1: right
	int num = 0;
	int x = 0;
	int y = 0;
	//face North
	int dx = 0;
	int dy = 1;
	while ( (c = fgetc(stdin)) != EOF) {
		if (c == 'L')
			dir = 1;
		else if (c == 'R')
			dir = -1;
		else if (c >= '0' && c <= '9')
			num = num * 10 + c - '0';
		else if (num > 0) { // apply instruction
			// rotate
			int dx_next = -dir * dy;
			dy = dir * dx;
			dx = dx_next;
			// translate
			x += dx * num;
			y += dy * num;
			// reset
			num = 0;
		}
	}
	// abs
	x = x < 0 ? -x : x;
	y = y < 0 ? -y : y;
	printf("%d\n", x + y);
	return 0;
}
