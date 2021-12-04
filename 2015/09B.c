#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PLACES 25

// From projecteuler, problem 041.c
bool next_perm(int a[], int len) {
	// returns false when no next permutation exists
	// step 1: from right to left, find first element higher than its right neighbor: pivot = a[idxPivot]
	// step 2: sort elements to right of idxPivot (invert order in this case!)
	// step 3: from idxPivot upward, find first element > pivot: idxSwap
	// step 4: swap pivot with a[idxSwap]
	int idxPivot;
	for (idxPivot = len - 2; idxPivot >= 0; --idxPivot) {
		if (a[idxPivot] < a[idxPivot + 1]) {
			// invert order of elements idxPivot+1 ... len-1
			int t;
			int ii = idxPivot + 1;
			int jj = len - 1;
			while (jj > ii) {
				t = a[jj];
				a[jj--] = a[ii];
				a[ii++] = t;
			}
			// from idxPivot upward, find first element > pivot: ii
			for (ii = idxPivot + 1; a[ii] < a[idxPivot]; ++ii);
			// swap a[ii] with a[idxPivot]
			t = a[ii];
			a[ii] = a[idxPivot];
			a[idxPivot] = t;
			break;
		}
	}
	return (idxPivot >= 0);
}

int place_to_idx(char* place, char* places[]) {
	// if place not in places, adds it
	int ii;
	for (ii = 0; places[ii] != NULL; ++ii)
		if (!strcmp(places[ii], place))
			return ii;
	int l = strlen(place);
	places[ii] = malloc((l + 1) * sizeof(char));
	strcpy(places[ii], place);
	return ii;
}

void process_line(char* line, int* idx1, int* idx2, int* dist, char* places[]) {
	char* delim = " \t\n";
	*idx1 = place_to_idx(strtok(line, delim), places);
	strtok(NULL, delim); // "to"
	*idx2 = place_to_idx(strtok(NULL, delim), places);
	strtok(NULL, delim); // =
	*dist = atoi(strtok(NULL, delim));
}

void set_distance(int d[], int i1, int i2, int x) {
	d[i1 * MAX_PLACES + i2] = x;
	d[i2 * MAX_PLACES + i1] = x;
}

int get_distance(int d[], int i1, int i2) {
	if (i1 == i2)
		return 0;
	return d[i1 * MAX_PLACES + i2];
}

int calc_distance(int distances[], int order[], int len_order) {
	int totdist = 0;
	int ii = 0;
	for (ii = 1; ii < len_order; ++ii)
		totdist += get_distance(distances, order[ii - 1], order[ii]);
	return totdist;
}

void print_route(char* places[], int order[], int len_order) {
	int ii;
	for (ii = 0; ii < len_order - 1; ++ii)
		printf("%s -> ", places[order[ii]]);
	printf("%s\n", places[order[len_order - 1]]);
}

int main(int argc, char* argv[]) {
	char* places[MAX_PLACES + 1]; // nul terminated char* array
	int distances[MAX_PLACES * MAX_PLACES];
	int perm[MAX_PLACES];
	int ii;
	int idx1, idx2, dist;
	for (ii = 0; ii < MAX_PLACES + 1; ++ii)
		places[ii] = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		process_line(line, &idx1, &idx2, &dist, places);
		printf("%s -> %s: %d\n", places[idx1], places[idx2], dist);
		set_distance(distances, idx1, idx2, dist);
	}
	printf("\n");
	free(line);

	int nr_places = 0;
	while (places[nr_places] != NULL)
		++nr_places;

	// permute through all options brute force
	int max_distance = 0;
	int best_perm[MAX_PLACES];
	for (ii = 0; ii < nr_places; ++ii)
		perm[ii] = ii;
	do {
		int d = calc_distance(distances, perm, nr_places);
		if (d > max_distance) {
			max_distance = d;
			memcpy(best_perm, perm, nr_places * sizeof(int));
		}
	} while (next_perm(perm, nr_places));
	print_route(places, best_perm, nr_places);
	printf("%d\n", max_distance);

	// clean up
	for (ii = 0; places[ii] != NULL; ++ii)
		free(places[ii]);
	return 0;
}
