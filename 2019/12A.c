#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define NR_MOONS 4
#define NR_DIMS  3

#define ABS(a) (((a)<0)?-(a):(a))

struct moon {
	int x[NR_DIMS];
	int v[NR_DIMS];
};

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

void sim_step(struct moon* moons) {
	// apply gravity
	for (int ii = 0; ii < NR_MOONS - 1; ++ii) {
		for (int jj = ii + 1; jj < NR_MOONS; ++jj) {
			for (int dim = 0; dim < NR_DIMS; ++dim) {
				int dx = moons[jj].x[dim] - moons[ii].x[dim];
				int dv = dx > 0 ? 1 : dx == 0 ? 0 : -1;
				moons[ii].v[dim] += dv;
				moons[jj].v[dim] -= dv;
			}
		}
	}
	// apply velocity
	for (int ii = 0; ii < NR_MOONS; ++ii) {
		for (int dim = 0; dim < NR_DIMS; ++dim) {
			moons[ii].x[dim] += moons[ii].v[dim];
		}
	}
}

int calc_energy(struct moon* moons) {
	int tot = 0;
	for (int ii = 0; ii < NR_MOONS; ++ii) {
		int pot = 0;
		int kin = 0;
		for (int dim = 0; dim < NR_DIMS; ++dim) {
			pot += ABS(moons[ii].x[dim]);
			kin += ABS(moons[ii].v[dim]);
		}
		tot += pot * kin;
	}
	return tot;
}

void print_moons(struct moon* moons) {
	for (int ii = 0; ii < NR_MOONS; ++ii) {
		printf("pos=<x=%3d, y=%3d, z=%3d, ", moons[ii].x[0], moons[ii].x[1], moons[ii].x[2]);
		printf(", vel=<x=%3d, y=%3d, z=%3d>\n", moons[ii].v[0], moons[ii].v[1], moons[ii].v[2]);
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
	//print_moons(moons);
	for (int step = 1; step <= 1000; ++step) {
		sim_step(moons);
		/*
		printf("After %d steps\n", step);
		print_moons(moons);
		printf("\n");
		*/
	}
	printf("%d\n", calc_energy(moons));

	return 0;
}
