#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINES 2048

int cmp_lines(const void* a, const void* b) {
	return strcmp(*(char**)a, *(char**)b);
}

struct guard {
	int           id;
	int           sleep[60];
	struct guard* next;
};

struct guard* extract_sleep_times(char** lines, int nr_lines) {
	struct guard* guards = NULL;
	struct guard* g = NULL;
	for (int ii = 0; ii < nr_lines; ++ii) {
		int tsleep, twake;
		if (strstr(lines[ii], "Guard")) { // start new guard info
			int id;
			char* l = strchr(lines[ii], '#') + 1;
			sscanf(l, "%d", &id);
			// try to find guard
			g = guards;
			while (g && g->id != id)
				g = g->next;
			if (!g) {
				g = malloc(sizeof(struct guard));
				g->id = id;
				for (int ii = 0; ii < 60; ++ii)
					g->sleep[ii] = 0;
				g->next = guards;
				guards = g;
			}
		}
		else if (strstr(lines[ii], "falls")) {
			char* l = strchr(lines[ii], ':') + 1;
			sscanf(l, "%d", &tsleep);
		}
		else if (strstr(lines[ii], "wakes")) {
			char* l = strchr(lines[ii], ':') + 1;
			sscanf(l, "%d", &twake);
			for (int t = tsleep; t < twake; ++t)
				++g->sleep[t];
		}
	}
	return guards;
}

int main(int argc, char* argv[]) {
	char* lines[MAX_LINES];
	int nr_lines = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		lines[nr_lines++] = line;
		line = NULL; // next call to getline will alloc new mem
	}
	// sort lines by time
	qsort(lines, nr_lines, sizeof(char*), cmp_lines);

	struct guard* guards = extract_sleep_times(lines, nr_lines);

	// post proc to find answer
	int most_minutes = 0;
	int minute = 0;
	int id = 0;
	for (struct guard* g = guards; g; g = g->next) {
		int maxnr = 0;
		int minute_maxnr = 0;
		for (int ii = 0; ii < 60; ++ii) {
			if (g->sleep[ii] > maxnr) {
				maxnr = g->sleep[ii];
				minute_maxnr = ii;
			}
		}
		if (maxnr > most_minutes) {
			most_minutes = maxnr;
			minute = minute_maxnr;
			id = g->id;
		}
	}
	printf("%d\n", id * minute);

	while (guards) {
		struct guard* g = guards;
		guards = g->next;
		free(g);
	}
	for (int ii = 0; ii < nr_lines; ++ii)
		free(lines[ii]);
	return 0;
}
