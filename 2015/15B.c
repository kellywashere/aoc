#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INGR 25
#define NR_PROP 5
#define TARGET_CAL 500

struct ingredient {
	int prop[NR_PROP];
};

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

char* find_nr(char* str) {
	while (*str && *str != '-' && (*str < '0' || *str > '9'))
		++str;
	return *str == '\0' ? NULL : str;
}

void process_line(char* line, struct ingredient* ing) {
	char* s = line;
	for (int ii = 0; ii < NR_PROP; ++ii) {
		s = find_nr(s);
		ing->prop[ii] = strtol(s, &s, 10);
	}
}

bool next_split_sum(int* a, int len) {
	// creates next iteration where all a[0..len-1] sum to same nr
	// as on entry. 
	int ii, jj;
	// find first non-zero nr from right, but not rightmost itself
	for (ii = len - 2; ii >= 0; --ii)
		if (a[ii] > 0)
			break;
	if (ii < 0)
		return false; // no next
	--a[ii++];
	// sum a[ii .. len-1]
	int sum = 1; // to compensate for --a[ii]
	for (jj = ii; jj < len; ++jj)
		sum += a[jj];
	a[ii++] = sum;
	for (jj = ii; jj < len; ++jj)
		a[jj] = 0;
	return true;
}

/*
void test_split_sum() {
#define N 4
	int ss[N];
	ss[0] = 100;
	for (int ii = 1; ii < N; ++ii)
		ss[ii] = 0;
	do {
		for (int ii = 0; ii < N; ++ii)
			printf("%d ", ss[ii]);
		printf("\n");
	} while (next_split_sum(ss, N));
}
*/

int main(int argc, char* argv[]) {
	int tot_tsp = 100;
	if (argc > 1)
		tot_tsp = atoi(argv[1]);
	struct ingredient ing[MAX_INGR];
	int nr_ing = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			process_line(line, &ing[nr_ing]);
			++nr_ing;
		}
	}
	free(line);

	int* tsp = calloc(nr_ing, sizeof(int));
	tsp[0] = tot_tsp;
	int best_score = 0;
	do {
		// count calories
		int cal = 0;
		for (int ingidx = 0; ingidx < nr_ing; ++ingidx)
			cal += tsp[ingidx] * ing[ingidx].prop[NR_PROP - 1];
		if (cal == TARGET_CAL) {
			int totscore = 1;
			for (int propidx = 0; propidx < NR_PROP - 1; ++propidx) {//-1: no calories
				int totp = 0;
				for (int ingidx = 0; ingidx < nr_ing; ++ingidx)
					totp += tsp[ingidx] * ing[ingidx].prop[propidx];
				totp = totp < 0 ? 0 : totp;
				totscore *= totp;
			}
			best_score = totscore > best_score ? totscore : best_score;
		}
	} while (next_split_sum(tsp, nr_ing));
	printf("%d\n", best_score);

	free(tsp);
	return 0;
}
