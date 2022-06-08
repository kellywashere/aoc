#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_COMPONENTS 64

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

struct comp {
	int p[2];
	int mid; // used to keep track of internal ports of connected components
	bool used;
};

int count_components_with_port(struct comp* comps, int nr_comps, int p) {
	int count = 0;
	for (int ii = 0; ii < nr_comps; ++ii)
		count += (comps[ii].p[0] == p || comps[ii].p[1] == p) ? 1 : 0;
	return count;
}

void order_ports(struct comp* c) {
	// p[0] should be <= p[1]
	int ps = c->p[0] < c->p[1] ? c->p[0] : c->p[1];
	int pl = c->p[0] < c->p[1] ? c->p[1] : c->p[0];
	c->p[0] = ps;
	c->p[1] = pl;
}

int find_largest_bridge(struct comp* comps, int nr_comps, int p, int sum) {
	int largest_sum = sum;
	for (int ii = 0; ii < nr_comps; ++ii) {
		if (comps[ii].used)
			continue;
		for (int portnr = 0; portnr <= 1; ++portnr) {
			if (comps[ii].p[portnr] == p) {
				comps[ii].used = true;
				int l = find_largest_bridge(comps, nr_comps, comps[ii].p[1 - portnr], sum + comps[ii].p[0] + comps[ii].mid + comps[ii].p[1]);
				if (l > largest_sum)
					largest_sum = l;
				comps[ii].used = false;
			}
		}
	}
	return largest_sum;
}

bool remove_unmatched_components(struct comp* comps, int* nr_comps) {
	bool changed = false;
	for (int ii = 0; ii < *nr_comps; ++ii) {
		if (comps[ii].p[0] == 0)
			continue;
		int count0 = count_components_with_port(comps, *nr_comps, comps[ii].p[0]);
		int count1 = count_components_with_port(comps, *nr_comps, comps[ii].p[1]);
		if (count0 == 1 && count1 == 1) {
			changed = true;
			--(*nr_comps);
			for (int jj = ii; jj < *nr_comps; ++jj)
				comps[jj] = comps[jj + 1];
		}
	}
	return changed;
}

bool connect_only_options(struct comp* comps, int* nr_comps) {
	bool changed = false;
	for (int portnr = 0; portnr <= 1; ++portnr) {
		for (int ii = 0; ii < *nr_comps; ++ii) {
			int p = comps[ii].p[portnr];
			if (p != 0) {
				int count = count_components_with_port(comps, *nr_comps, p);
				if (count == 2) { // find the idx of the other one
					int idx = ii + 1;
					for ( ; idx < *nr_comps; ++idx)
						if (comps[idx].p[0] == p || comps[idx].p[1] == p)
							break;
					if (idx < *nr_comps) {
						changed = true;
						comps[ii].p[portnr] = comps[idx].p[comps[idx].p[0] == p ? 1 : 0];
						order_ports(&comps[ii]);
						comps[ii].mid += 2 * p + comps[idx].mid;
						--(*nr_comps);
						for (; idx < *nr_comps; ++idx)
							comps[idx] = comps[idx + 1];
					}
				}
			}
		}
	}
	return changed;
}

int main(int argc, char* argv[]) {
	struct comp comps[MAX_COMPONENTS];
	int nr_comps = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		read_int(&l, &comps[nr_comps].p[0]);
		read_int(&l, &comps[nr_comps].p[1]);
		order_ports(&comps[nr_comps]);
		comps[nr_comps].mid = 0;
		comps[nr_comps].used = false;
		++nr_comps;
	}
	free(line);

	// simplify
	bool changed = true;
	while (changed) {
		changed = remove_unmatched_components(comps, &nr_comps);
		changed = connect_only_options(comps, &nr_comps) || changed;
	}

	int largest_sum = find_largest_bridge(comps, nr_comps, 0, 0);
	printf("%d\n", largest_sum);

	return 0;
}
