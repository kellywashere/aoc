#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// y-trajectory: vy0 i initial speed in y-dir. Assume vy0 > 0.
// height reached: vy0 + (vy0 - 1) + ... + 1 + 0 = vy0*(vy0 + 1)/2
// After 2*vy0 + 1 steps, we reach y = 0 again, with speed -vy0 - 1
// next ypos: -vy0 - 1
// So upper limit vy0: -vy0 - 1 >= target_bottom --> vy0 <= -target_bottom - 1
// (asumption: target has negative y-values)

// y(n) = y0 + (vy0 - 0) + (vy0 - 1) + (vy0 - 2) ... + (vy0 - (n - 1)) =
//      = n * vy0 - (n-1)*n/2 = n * vy0 - 1/2*(n^2 - n) = -n^2/2 + (vy0 + 1/2) * n

// x(n) = vx0 + (vx0 - 1) + (vx0 - 2) + ... (vx0 - (n-1)) for n <= vx0 - 1
// x(n) = n * vx0 - (n-1)*n/2 for n <= vx0 - 1
// x(n) = vx0*(vx0 + 1) / 2 for n > vx0 - 1
// upper limit vx0: vx0 <= target_right

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "Could not allocate %zu bytes of memory\n", size);
	return p;
}

double abc(double a, double b, double c) {
	// only + solution is relevant here
	return (-b + sqrt(b*b - 4*a*c))/(2*a);
}

int main(int argc, char* argv[]) {
	int target_left, target_right, target_top, target_bottom;

	scanf("target area: x=%d..%d, y=%d..%d", &target_left, &target_right, &target_bottom, &target_top);
	// printf("%d, %d, %d, %d\n", target_left, target_right, target_bottom, target_top);

	int count = 0;
	int vy_max = -target_bottom - 1;
	for (int vy0 = vy_max; vy0 >= target_bottom; --vy0) {
		// find smallest n s.t. y(n) <= target_top
		// n^2 - (2*vy0 + 1) * n + 2 * target_top >= 0
		double nd = abc(1.0, -(2*vy0 + 1), 2*target_top);
		int n = nd; // we should really have ceil(nd)
		if (n * vy0 - (n-1)*n/2 > target_top)
			++n;
		int y = n * vy0 - (n-1) * n / 2;
		if (y <= target_top && y >= target_bottom) {
			int n_min = n;
			int n_max = n;
			for (; n * vy0 - (n-1)*n/2 >= target_bottom; ++n)
				n_max = n;
			for (int vx0 = target_right; vx0 >= target_left / n; --vx0) {
				bool target_hit = false;
				for (n = n_min; !target_hit && n <= n_max; ++n) {
					int x = vx0*(vx0 + 1) / 2;
					if (n < vx0 - 1)
						x = n * vx0 - (n-1)*n/2;
					if (x >= target_left && x <= target_right)
						target_hit = true;
				}
				if (target_hit)
					++count;
			}
		}
	}
	printf("%d\n", count);
	return 0;
}
