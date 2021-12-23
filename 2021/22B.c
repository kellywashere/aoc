#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

struct cube {
	int x[2];
	int y[2];
	int z[2];
	struct cube* next;
};

struct cube* create_cube(int x0, int x1, int y0, int y1, int z0, int z1) {
	struct cube* c = mymalloc(sizeof(struct cube));
	c->x[0] = x0;
	c->x[1] = x1;
	c->y[0] = y0;
	c->y[1] = y1;
	c->z[0] = z0;
	c->z[1] = z1;
	return c;
}

bool intersect(struct cube* c1, struct cube* c2) {
	if (c2->x[1] < c1->x[0] || (c2->x[0] > c1->x[1]))
		return false;
	if (c2->y[1] < c1->y[0] || (c2->y[0] > c1->y[1]))
		return false;
	if (c2->z[1] < c1->z[0] || (c2->z[0] > c1->z[1]))
		return false;
	return true;
}

void read_coords(char** line, int* w1, int* w2) {
	*w1 = strtol(*line, line, 10);
	*line += 2;
	*w2 = strtol(*line, line, 10);
}

void print_cube(struct cube* c) {
	printf("  %d..%d, %d..%d, %d..%d\n", c->x[0], c->x[1], c->y[0], c->y[1], c->z[0], c->z[1]);
}

void process_line(char* line, struct cube** cubes) {
	struct cube* cube = mymalloc(sizeof(struct cube));
	cube->next = NULL;
	int turnon = false;
	if (line[1] == 'n') { // on
		turnon = true;
		line += 5;
	}
	else
		line += 6;
	read_coords(&line, &cube->x[0], &cube->x[1]);
	line += 3;
	read_coords(&line, &cube->y[0], &cube->y[1]);
	line += 3;
	read_coords(&line, &cube->z[0], &cube->z[1]);
	if ((cube->x[1] < -50) || (cube->x[0] > 50))
		return;
	if ((cube->y[1] < -50) || (cube->y[0] > 50))
		return;
	if ((cube->z[1] < -50) || (cube->z[0] > 50))
		return;

	struct cube* c = *cubes;
	struct cube* prev = NULL;
	while (c) {
		if (intersect(c, cube)) {
			// chop of off pieces of c that do not intersect with cube
			if (cube->x[0] > c->x[0]) {
				// split c into s (no overap with cube) and c (overlap)
				struct cube* s = create_cube(c->x[0], cube->x[0] - 1, c->y[0], c->y[1], c->z[0], c->z[1]);
				c->x[0] = cube->x[0];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			if (cube->x[1] < c->x[1]) {
				struct cube* s = create_cube(cube->x[1] + 1, c->x[1], c->y[0], c->y[1], c->z[0], c->z[1]);
				c->x[1] = cube->x[1];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			if (cube->y[0] > c->y[0]) {
				struct cube* s = create_cube(c->x[0], c->x[1], c->y[0], cube->y[0] - 1, c->z[0], c->z[1]);
				c->y[0] = cube->y[0];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			if (cube->y[1] < c->y[1]) {
				struct cube* s = create_cube(c->x[0], c->x[1], cube->y[1] + 1, c->y[1], c->z[0], c->z[1]);
				c->y[1] = cube->y[1];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			if (cube->z[0] > c->z[0]) {
				struct cube* s = create_cube(c->x[0], c->x[1], c->y[0], c->y[1], c->z[0], cube->z[0] - 1);
				c->z[0] = cube->z[0];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			if (cube->z[1] < c->z[1]) {
				struct cube* s = create_cube(c->x[0], c->x[1], c->y[0], c->y[1], cube->z[1] + 1, c->z[1]);
				c->z[1] = cube->z[1];
				s->next = *cubes;
				*cubes = s;
				if (!prev) prev = s;
			}
			// remove c from list
			if (!prev) { // only happens when first cube on list is subset of second one
				*cubes = c->next;
				free(c);
				c = *cubes; // next iter
			}
			else {
				prev->next = c->next;
				free(c);
				c = prev->next;
			}
		}
		else {
			prev = c;
			c = c->next;
		}
	}
	if (turnon) {
		cube->next = *cubes;
		*cubes = cube;
	}
}

int count_cubes(struct cube* cubes) {
	int count = 0;
	while (cubes) {
		int x = cubes->x[1] - cubes->x[0] + 1;
		int y = cubes->y[1] - cubes->y[0] + 1;
		int z = cubes->z[1] - cubes->z[0] + 1;
		count += x * y * z;
		cubes = cubes->next;
	}
	return count;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct cube* cubes = NULL;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		process_line(line, &cubes);
	}
	printf("%d\n", count_cubes(cubes));
	// clean up
	while (cubes) {
		struct cube* c = cubes;
		cubes = c->next;
		// print_cube(c);
		free(c);
	}
	free(line);
	return 0;
}
