// OMG this one was the hardest I have solved so far!!!
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(a) ((a)<0?(-(a)):(a))

struct bot {
	int         pos[3]; // x,y,z
	int         r;
	struct bot* next;
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

struct octahedron {
	int center[3];
	int radius;
	int dist; // smallest distance to origin
};

bool octahedrons_overlap(struct octahedron* oh1, struct octahedron* oh2) {
	if (oh1->radius < 0 || oh2->radius < 0)
		return false;
	int dx = ABS(oh1->center[0] - oh2->center[0]);
	int dy = ABS(oh1->center[1] - oh2->center[1]);
	int dz = ABS(oh1->center[2] - oh2->center[2]);
	return (dx + dy + dz) <= oh1->radius + oh2->radius;
}

void calc_octahedron_dist(struct octahedron* oh) {
	// this obvious equation took me so long to derive ... :(
	// oh->dist = MAX(0, ABS(oh->center[0]) + ABS(oh->center[1]) + ABS(oh->center[2]) - oh->radius);
	int d = ABS(oh->center[0]) + ABS(oh->center[1]) + ABS(oh->center[2]) - oh->radius;
	oh->dist = MAX(0, d);
}

int main(int argc, char* argv[]) {
	struct bot* bots = NULL;
	int nr_bots = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		struct bot* bot = malloc(sizeof(struct bot));
		char* l = line;
		for (int dim = 0; dim < 3; ++dim)
			bot->pos[dim] = read_int(&l);
		bot->r = read_int(&l);
		bot->next = bots;
		bots = bot;
		++nr_bots;
	}
	free(line);

	// create octahedrons, from bots
	struct octahedron* ohs = malloc(nr_bots * sizeof(struct octahedron));
	int idx = 0;
	for (struct bot* b = bots; b; b = b->next) {
		for (int dim = 0; dim < 3; ++dim)
			ohs[idx].center[dim] = b->pos[dim];
		ohs[idx].radius = b->r;
		calc_octahedron_dist(&ohs[idx]);
		++idx;
	}
	int nr_ohs = nr_bots;

	// check overlaps
	bool* overlaps = calloc(nr_ohs * nr_ohs, sizeof(bool));
	int* nr_overlaps = calloc(nr_ohs, sizeof(int));
	// overlap with itself:
	for (int ii = 0; ii < nr_ohs; ++ii) {
		overlaps[ii * nr_ohs + ii] = true;
		++nr_overlaps[ii];
	}
	// overlap with others:
	for (int ii = 0; ii < nr_ohs - 1; ++ii) {
		for (int jj = ii + 1; jj < nr_ohs; ++jj) {
			if (octahedrons_overlap(&ohs[ii], &ohs[jj])) {
				overlaps[ii * nr_ohs + jj] = true;
				overlaps[jj * nr_ohs + ii] = true;
				++nr_overlaps[ii];
				++nr_overlaps[jj];
			}
		}
	}
	/*
	for (int ii = 0; ii < nr_ohs; ++ii)
		printf("oct %d ovl w %d\n", ii, nr_overlaps[ii]);
	*/

	// Algo to find region with most overlaps (not guaranteed to always work I think)

	// pick not forbidden and not taken row with most `true` (> 0)
	//   ties resolved by smallest distance to origin
	// mark row as taken
	// for each `false`in that row, mark corresponding octahedron forbidden
	// repeat until no row left in criterion

	bool* taken = calloc(nr_ohs, sizeof(bool));
	int nr_taken = 0;
	bool* forbidden = calloc(nr_ohs, sizeof(bool));
	int nr_forbidden = 0;
	while (nr_taken + nr_forbidden < nr_ohs) {
		int max_overlaps = 0;
		int max_row = 0;
		for (int r = 0; r < nr_ohs; ++r) {
			if (!taken[r] && !forbidden[r] && nr_overlaps[r] > 0) {
				if (nr_overlaps[r] > max_overlaps || (nr_overlaps[r] == max_overlaps && ohs[r].dist < ohs[max_row].dist)) {
					max_overlaps = nr_overlaps[r];
					max_row = r;
				}
			}
		}
		if (max_overlaps == 0)
			break;
		// printf("Taking octahedron %d with %d overlaps\n", max_row, max_overlaps);
		taken[max_row] = true;
		++nr_taken;
		for (int ii = 0; ii < nr_ohs; ++ii) { // go over row find `false`s
			if (!overlaps[max_row * nr_ohs + ii]) {
				if (!forbidden[ii]) {
					forbidden[ii] = true; // we canot pick octahedron ii, it does not overlap with the current one
					++nr_forbidden;
				}
				// all rows containing ref to ii should set that entry to false, it does not matter anymore
				for (int r = 0; r < nr_ohs; ++r) {
					if (overlaps[r * nr_ohs + ii]) {
						overlaps[r * nr_ohs + ii] = false;
						--nr_overlaps[r];
					}
				}
			}
		}
	}

	// the next part of the algo took me a while to think of. No need to calculate real intersection between
	// all ohs. We can take max of all dists as the answer. This is easier to understand in a 2D image with
	// 45deg rotated squares, rather than in 3D...
	int closest_dist = 0;
	for (int ii = 0; ii < nr_ohs; ++ii) {
		if (taken[ii])
			closest_dist = MAX(ohs[ii].dist, closest_dist);
	}
	printf("%d\n", closest_dist);


	// free bots
	while (bots) {
		struct bot* b = bots;
		bots = bots->next;
		free(b);
	}
	free(ohs);
	free(overlaps);
	free(nr_overlaps);
	free(taken);
	free(forbidden);

	return 0;
}
