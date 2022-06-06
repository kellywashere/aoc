#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PARTICLES 1024

struct vector {
	int x;
	int y;
	int z;
};

struct particle {
	int           id;
	struct vector p;
	struct vector v;
	struct vector a;
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

int manchester_len(struct vector* v) {
	int len = v->x < 0 ? -v->x : v->x;
	len += v->y < 0 ? -v->y : v->y;
	len += v->z < 0 ? -v->z : v->z;
	return len;
}

int cmp_vectors(struct vector* a, struct vector* b) {
	// compares "manchester length"
	return manchester_len(a) - manchester_len(b);
}

int cmp_particles_on_a(const void* a, const void* b) {
	struct particle* pa = (struct particle*)a;
	struct particle* pb = (struct particle*)b;
	return cmp_vectors(&pa->a, &pb->a);
}

int main(int argc, char* argv[]) {
	struct particle particles[MAX_PARTICLES];
	int nrparticles = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		particles[nrparticles].id = nrparticles;
		read_vector(&l, &particles[nrparticles].p);
		read_vector(&l, &particles[nrparticles].v);
		read_vector(&l, &particles[nrparticles].a);
		++nrparticles;
	}
	qsort(particles, nrparticles, sizeof(struct particle), cmp_particles_on_a);
	/*
	for (int ii = 0; ii < nrparticles; ++ii) {
		printf("%d: p=<%d,%d,%d>, v=<%d,%d,%d>, a=<%d,%d,%d>\n", 
				particles[ii].id,
				particles[ii].p.x, particles[ii].p.y, particles[ii].p.z,
				particles[ii].v.x, particles[ii].v.y, particles[ii].v.z,
				particles[ii].a.x, particles[ii].a.y, particles[ii].a.z);
	}
	*/
	printf("%d\n", particles[0].id); // We are lucky in this problem: unique min |a| found...
	free(line);
	return 0;
}
