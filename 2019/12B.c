#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define NR_MOONS 4
#define NR_DIMS  3

#define ABS(a) (((a)<0)?-(a):(a))

struct moon {
	int x[NR_DIMS];
	int v[NR_DIMS];
};

int64_t gcd(int64_t a, int64_t b) {
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;
	while (a != 0 && b != 0) {
		int64_t na = b;
		b = a % b;
		a = na;
	}
	return b == 0 ? a : b;
}

int64_t lcm(int64_t a, int64_t b) {
	a = ABS(a);
	b = ABS(b);
	return a * b / gcd(a, b);
}

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

void sim_step_dim(struct moon* moons, int dim) {
	// apply gravity
	for (int ii = 0; ii < NR_MOONS - 1; ++ii) {
		for (int jj = ii + 1; jj < NR_MOONS; ++jj) {
			int dx = moons[jj].x[dim] - moons[ii].x[dim];
			int dv = dx > 0 ? 1 : dx == 0 ? 0 : -1;
			moons[ii].v[dim] += dv;
			moons[jj].v[dim] -= dv;
		}
	}
	// apply velocity
	for (int ii = 0; ii < NR_MOONS; ++ii)
		moons[ii].x[dim] += moons[ii].v[dim];
}

bool eq_state_dim(struct moon* m1, struct moon* m2, int dim) {
	bool eq = true;
	for (int ii = 0; eq && ii < NR_MOONS; ++ii)
		eq = eq && m1[ii].x[dim] == m2[ii].x[dim] && m1[ii].v[dim] == m2[ii].v[dim];
	return eq;
}

void copy_state(struct moon* dest, struct moon* src) {
	memcpy(dest, src, NR_MOONS * sizeof(struct moon));
}

void floyd_dim(struct moon* moons, int dim, int* mu, int* lambda) {
	// https://en.wikipedia.org/wiki/Cycle_detection
	struct moon tortoise[4];
	struct moon hare[4];
	copy_state(tortoise, moons);
	sim_step_dim(tortoise, dim); // tortoise = f(x0)

	copy_state(hare, tortoise);
	sim_step_dim(hare, dim); // hare = f(f(x0))

	while (!eq_state_dim(tortoise, hare, dim)) {
		sim_step_dim(tortoise, dim); // tortoise = f(tortoise)
		sim_step_dim(hare, dim);
		sim_step_dim(hare, dim); // hare = f(f(hare))
	}

	*mu = 0;
	copy_state(tortoise, moons); // tortoise = x0
	while (!eq_state_dim(tortoise, hare, dim)) {
		sim_step_dim(tortoise, dim); // tortoise = f(tortoise)
		sim_step_dim(hare, dim); // hare = f(hare)
		++(*mu);
	}

	*lambda = 1;
	copy_state(hare, tortoise);
	sim_step_dim(hare, dim); // hare = f(tortoise)
	while (!eq_state_dim(tortoise, hare, dim)) {
		sim_step_dim(hare, dim); // hare = f(hare)
		++(*lambda);
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct moon moons[NR_MOONS];
	for (int ii = 0; ii < NR_MOONS; ++ii) {
		getline(&line, &len, stdin);
		char* l = line;
		for (int dim = 0; dim < NR_DIMS; ++dim) {
			moons[ii].x[dim] = read_int(&l);
			moons[ii].v[dim] = 0;
		}
	}
	free(line);

	int mu[NR_DIMS];
	int lambda[NR_DIMS];
	for (int dim = 0; dim < NR_DIMS; ++dim) {
		floyd_dim(moons, dim, &mu[dim], &lambda[dim]);
		//printf("Dim %d: mu = %d, lambda = %d\n", dim, mu[dim], lambda[dim]);
	}

	int64_t answer = lambda[0];
	for (int dim = 1; dim < NR_DIMS; ++dim)
		answer = lcm(answer, lambda[dim]);
	printf("%" PRId64 "\n", answer);

	return 0;
}
