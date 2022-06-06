#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#define MAX_PARTICLES 1024

struct vector {
	int x;
	int y;
	int z;
};

struct particle {
	struct vector p;
	struct vector v;
	struct vector a;
	int    n_collision; // n is time here...
	bool   removed;
};

bool read_int(char** pLine, int* x) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && *line != '-' && !isdigit(*line))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
	*pLine = line;
	return true;
}

bool read_vector(char** pLine, struct vector* v) {
	struct vector vec;
	char* line = *pLine;
	while (*line && *line != '<')
		++line;
	if (*line != '<')
		return false;
	if (!read_int(&line, &vec.x))
		return false;
	if (!read_int(&line, &vec.y))
		return false;
	if (!read_int(&line, &vec.z))
		return false;
	while (*line && *line != '>')
		++line;
	if (*line != '>')
		return false;
	*v = vec;
	*pLine = line + 1;
	return true;
}

bool check_collision(struct particle* pa, struct particle* pb, int n) {
	int dx = pa->p.x - pb->p.x + n*(pa->v.x - pb->v.x) + n*(n+1)/2*(pa->a.x - pb->a.x);
	int dy = pa->p.y - pb->p.y + n*(pa->v.y - pb->v.y) + n*(n+1)/2*(pa->a.y - pb->a.y);
	int dz = pa->p.z - pb->p.z + n*(pa->v.z - pb->v.z) + n*(n+1)/2*(pa->a.z - pb->a.z);
	return dx == 0 && dy == 0 && dz == 0;
}

bool collide(struct particle* pa, struct particle* pb, int* n_coll) {
	int a = pa->a.x - pb->a.x;
	int b = 2*(pa->v.x - pb->v.x) + a;
	int c = 2*(pa->p.x - pb->p.x);
	if (a < 0) { // easier to find lowest value of n first this way
		a = -a;
		b = -b;
		c = -c;
	}
	if (a == 0) { // linear equation
		if (b != 0 && c % b == 0) {
			int n = -c/b;
			if (n >= 0 && check_collision(pa, pb, n)) {
				*n_coll = n;
				return true;
			}
		}
	}
	else {
		// n = (-b +- sqrt(b^2 - 4ac))/(2a)
		int D = b * b - 4 * a * c;
		if (D >= 0) {
			int sD = (int)floor(sqrt(D) + 0.5);
			if (sD * sD == D) {
				for (int s = -1; s <= 1; s += 2) { // -/+
					int num = s * sD - b;
					if (num >= 0 && num % (2*a) == 0) {
						int n = num/(2*a);
						if (check_collision(pa, pb, n)) {
							*n_coll = n;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

int main(int argc, char* argv[]) {
	struct particle particles[MAX_PARTICLES];
	int nrparticles = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		read_vector(&l, &particles[nrparticles].p);
		read_vector(&l, &particles[nrparticles].v);
		read_vector(&l, &particles[nrparticles].a);
		particles[nrparticles].removed = false;
		particles[nrparticles].n_collision = -1;
		++nrparticles;
	}
	bool changed = true;
	int remaining = nrparticles;
	while (changed) {
		changed = false;
		// step 1: found minimum n at which collision happens
		int nmin = INT_MAX;
		for (int ii = 0; ii < nrparticles - 1; ++ii) {
			if (particles[ii].removed)
				continue;
			for (int jj = ii + 1; jj < nrparticles; ++jj) {
				if (particles[jj].removed)
					continue;
				int n;
				if (collide(&particles[ii], &particles[jj], &n)) {
					// printf("%d and %d collide on n=%d\n", ii, jj, n);
					if (n <= nmin) {
						nmin = n;
						particles[ii].n_collision = n;
						particles[jj].n_collision = n;
					}
				}
			}
		}
		// step 2: remove all particles colliding at nmin
		for (int ii = 0; ii < nrparticles; ++ii) {
			if (!particles[ii].removed && particles[ii].n_collision == nmin) {
				particles[ii].removed = true;
				--remaining;
				changed = true;
			}
		}
	}
	printf("%d\n", remaining);
	free(line);
	return 0;
}
