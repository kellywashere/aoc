#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_ATOMS 26

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "Could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

int atom_to_idx(char* a, char* atoms[]) {
	// if a not in atoms array, adds it
	int ii;
	for (ii = 0; atoms[ii] != NULL; ++ii)
		if (!strcmp(atoms[ii], a))
			return ii;
	int l = strlen(a);
	atoms[ii] = malloc((l + 1) * sizeof(char));
	strcpy(atoms[ii], a);
	atoms[ii + 1] = NULL;
	return ii;
}

char* molecule_long_to_short_name(char* lstr, char* atoms[]) {
	char latom[3];
	int latom_idx = 0;
	char* sstr = malloc((strlen(lstr) + 1) * sizeof(char));
	int sstr_idx = 0;
	while (isalpha(*lstr)) {
		latom[latom_idx++] = *lstr++;
		if (latom_idx == 2 || !(*lstr >= 'a' && *lstr <= 'z')) { // end of atom
			latom[latom_idx] = '\0';
			sstr[sstr_idx++] = atom_to_idx(latom, atoms) + 'a';
			latom_idx = 0;
		}
	}
	sstr[sstr_idx] = '\0';
	return sstr;
}

struct rule {
	char  a; // atom (short name)
	char* m; // molecule (short atom names)
	struct rule* next; // LL option
};

void print_rules(struct rule* rules) {
	while (rules) {
		printf("%c->%s\n", rules->a, rules->m);
		rules = rules->next;
	}
}

struct rule* mergesort_rules(struct rule* ll) {
	if (!ll || ll->next == NULL)
		return ll;
	// ll has at least 2 elements
	struct rule* e;
	struct rule* ll_split[2];
	ll_split[0] = ll_split[1] = NULL;
	// split list in two parts
	int lidx = 0;
	while (ll) {
		e = ll;
		ll = ll->next;
		e->next = ll_split[lidx];
		ll_split[lidx] = e;
		lidx = 1 - lidx;
	}
	// sort both parts using recursion
	ll_split[0] = mergesort_rules(ll_split[0]);
	ll_split[1] = mergesort_rules(ll_split[1]);
	// merge
	struct rule* tail = NULL;
	while (ll_split[0] != NULL && ll_split[1] != NULL) {
		lidx = strlen(ll_split[0]->m) > strlen(ll_split[1]->m) ? 0 : 1;
		e = ll_split[lidx];
		ll_split[lidx] = ll_split[lidx]->next;
		e->next = NULL;
		if (!ll)
			ll = e;
		if (tail)
			tail->next = e;
		tail = e;
	}
	if (ll_split[0])
		tail->next = ll_split[0];
	else
		tail->next = ll_split[1];
	return ll;
}


struct rule* process_replacement(char* line, char* atoms[]) {
	struct rule* rule = mymalloc(sizeof(struct rule));
	//printf("%s", line);
	char* delim = " \n";
	char* t = strtok(line, delim);
	rule->a = atom_to_idx(t, atoms) + 'a';

	strtok(NULL, delim); // skip arrow
	t = strtok(NULL, delim);
	rule->m = molecule_long_to_short_name(t, atoms);

	return rule;
}

void apply_rule_inplace(char* m, int idx, struct rule* r) {
	// printf("Applying rule %c=>%s to %s @ %d\n", r->a, r->m, m, idx);
	// does not check if r->a actually can be found at m + idx
	m += idx;
	int lr = strlen(r->m);
	int lm = strlen(m);
	if (lr > 1)
		memmove(m + lr, m + 1, lm); // includes \0
	memcpy(m, r->m, lr);
	// printf("out molecule: %s\n", m - idx);
}

void apply_rule_backward_inplace(char* m, int idx, struct rule* r) {
	// printf("Applying backward rule %s=>%c to %s @ %d\n", r->m, r->a, m, idx);
	// does not check if r->m actually can be found at m + idx
	m += idx;
	int lr = strlen(r->m);
	int lm = strlen(m);
	if (lr > 1)
		memmove(m + 1, m + lr, lm - lr + 1); // +1 for \0
	m[0] = r->a;
	// printf("out molecule: %s\n", m - idx);
}


// debug:
int g_shortest_len;

int count_steps_rec(char* m, struct rule* rules, char* target, int depth, int* best_depth) {
	if (*best_depth > 0 && depth >= *best_depth)
		return -1;
	if (!strcmp(m, target)) {
		printf("Found target after %d steps\n", depth);
		*best_depth = depth;
		return depth;
	}
	
	int lm = strlen(m);
	if (g_shortest_len == -1 || lm < g_shortest_len) {
		g_shortest_len = lm;
		printf("Shortest len: %d\n", g_shortest_len);
	}

	struct rule* r = rules;
	int best = -1;
	while (r) {
		char* loc = strstr(m, r->m);
		if (loc) {
			apply_rule_backward_inplace(m, loc - m, r);
			int steps = count_steps_rec(m, rules, target, depth + 1, best_depth);
			if (steps >= 0) {
				if (best < 0 || steps < best)
					best = steps;
			}
			apply_rule_inplace(m, loc - m, r);
		}
		r = r->next;
	}
	return best;
}

int count_steps(char* m, struct rule* rules, char* target) {
	g_shortest_len = strlen(m);
	int best_depth = -1;
	return count_steps_rec(m, rules, target, 0, &best_depth);
}

int main(int argc, char* argv[]) {
	char* atoms[MAX_ATOMS + 1]; // nul terminated char* array with "long" atom names
	int ii;
	atoms[0] = NULL;

	struct rule* rules = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break; // next line is calibration molecule
		struct rule* r = process_replacement(line, atoms);
		// add to LL
		r->next = rules;
		rules = r;
	}
	rules = mergesort_rules(rules);

	char target[2]; // target string
	target[0] = atom_to_idx("e", atoms) + 'a';
	target[1] = '\0';
	printf("Target: %s\n", target);
	// read calibration modecule
	char* m = NULL;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		m = molecule_long_to_short_name(line, atoms);
	}
	free(line);

	int count = count_steps(m, rules, target);
	printf("%d\n", count);

	// clean up
	free(m);
	while (rules) {
		struct rule* r = rules;
		rules = r->next;
		free(r);
	}
	for (ii = 0; atoms[ii] != NULL; ++ii)
		free(atoms[ii]);
	return 0;
}
