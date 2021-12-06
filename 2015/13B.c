#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NAMES 25

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

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

int name_to_idx(char* name, char* names[]) {
	// if name not in names, adds it
	int ii;
	for (ii = 0; names[ii] != NULL; ++ii)
		if (!strcmp(names[ii], name))
			return ii;
	int l = strlen(name);
	names[ii] = malloc((l + 1) * sizeof(char));
	strcpy(names[ii], name);
	return ii;
}

void process_line(char* line, int* idx1, int* idx2, int* happy_gained, char* names[]) {
	char* t;
	int ii;
	bool is_neg = false;
	char* delim = " \t\n.";
	*idx1 = name_to_idx(strtok(line, delim), names);
	strtok(NULL, delim); // "would"
	t = strtok(NULL, delim); // gain / lose
	if (*t == 'l')
		is_neg = true;
	int x = atoi(strtok(NULL, delim)); // number
	*happy_gained = is_neg ? -x : x;
	for (ii = 0; ii < 6; ++ii)
		strtok(NULL, delim); // "happiness units by sitting next to"
	*idx2 = name_to_idx(strtok(NULL, delim), names);
}

void set_happygain(int h[], int i1, int i2, int x) {
	h[i1 * MAX_NAMES + i2] = x;
}

int get_happygain(int h[], int i1, int i2) {
	return h[i1 * MAX_NAMES + i2];
}

int calc_happiness(int happygains[], int order[], int len_order) {
	int tot = 0;
	int ii;
	for (ii = 0; ii < len_order; ++ii) {
		int mid   = order[ii];
		int left  = order[(ii + len_order - 1) % len_order];
		int right = order[(ii + 1) % len_order];
		tot += get_happygain(happygains, mid, left);
		tot += get_happygain(happygains, mid, right);
	}
	return tot;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // nul terminated char* array
	int happygains[MAX_NAMES * MAX_NAMES];
	int perm[MAX_NAMES]; // note: we do not permutate p[0] (circular setup)
	int ii;
	int idx1, idx2, happy_gained;
	for (ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		process_line(line, &idx1, &idx2, &happy_gained, names);
		// printf("%s -> %s: %d\n", names[idx1], names[idx2], happy_gained);
		set_happygain(happygains, idx1, idx2, happy_gained);
	}
	free(line);

	// count names
	int nr_names = 0;
	while (names[nr_names] != NULL)
		++nr_names;

	// 13B: add myself
	for (ii = 0; ii < nr_names; ++ii) {
		set_happygain(happygains, ii, nr_names, 0);
		set_happygain(happygains, nr_names, ii, 0);
	}
	++nr_names;

	// permute through all options brute force
	int max_score = -666666;
	int best_perm[MAX_NAMES];
	for (ii = 0; ii < nr_names; ++ii)
		perm[ii] = ii;
	do {
		int h = calc_happiness(happygains, perm, nr_names);
		if (h > max_score) {
			max_score = h;
			memcpy(best_perm, perm, nr_names * sizeof(int));
		}
	} while (next_perm(perm + 1, nr_names - 1)); // we do not permutate p[0] (circular setup)
	// print_route(names, best_perm, nr_names);
	printf("%d\n", max_score);

	// clean up
	for (ii = 0; names[ii] != NULL; ++ii)
		free(names[ii]);
	return 0;
}
