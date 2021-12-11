/* Brute force method to at least produce the answer for reference */
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

struct rule* process_replacement(char* line, char* atoms[]) {
	struct rule* rule = mymalloc(sizeof(struct rule));
	//printf("%s", line);
	char* delim = " \n";
	char* t = strtok(line, delim);
	rule->a = atom_to_idx(t, atoms) + 'a';

	strtok(NULL, delim); // skip arrow
	t = strtok(NULL, delim);
	rule->m = molecule_long_to_short_name(t, atoms);

	printf("%c -> %s\n", rule->a, rule->m);
	return rule;
}

// 0 1   2    3 4 5      lm = 6, idx = 2
// 0 1 2a 2b 2c 3 4 5    lr = 3
char* make_replacement(char* m, int idx, struct rule* r) {
	int lr = strlen(r->m);
	int lm = strlen(m);
	char* newm = malloc((lm + lr) * sizeof(char));
	if (idx > 0)
		memcpy(newm, m, idx);
	memcpy(newm + idx, r->m, lr);
	memcpy(newm + idx + lr, m + idx + 1, lm - idx); // also copies \0
	return newm;
}

bool check_duplicate(char* m, int idx, struct rule* r, struct rule* rules, bool verbose) {
	int lr = strlen(r->m); // length of replacement
	if (lr == 1)
		return false; // replacements of len 1 are always unique
	if (r->m[lr - 1] != r->a)
		return false; // last atom of r->m needs to be equal to r->a to even consider duplicate
	// self-similarity of m when shifted by lr - 1
	int ls = 0;
	for (int ii = idx - 1; ii - (lr - 1) >= 0 && m[ii] == m[ii - (lr - 1)]; --ii)
		++ls;
	// max distance of prev atom to consider: lr + ls
	int maxdist = lr + ls;
	for (int dist = 1; dist <= maxdist && idx - dist >= 0; ++dist) {
		// check all rules for m[idx - dist]
		struct rule* rp = rules;
		while (rp) {
			// check if rule rp on m[idx-dist] leads to same string as r on m[idx]
			if (m[idx - dist] == rp->a && rp->m[0] == rp->a && strlen(rp->m) == lr) {
				// rule rp makes a chance to produce duplicate
				bool equal = true;
				// check overlap rp->m and m
				for (int jj = 1; equal && jj <= dist - 1 && jj < lr; ++jj)
					equal = equal && rp->m[jj] == m[idx - dist + jj];
				// check overlap rp->m and r->m
				for (int jj = 0; equal && jj + dist < lr; ++jj)
					equal = equal && rp->m[jj + dist] == r->m[jj];
				//check overlap m and r->m
				for (int jj = 1; equal && jj <= dist - 1 && lr - jj - 1 >= 0; ++jj)
					equal = equal && m[idx - jj] == r->m[lr - jj - 1];
				if (equal)  { // found replication
					if (verbose) {
						printf("relevant substring: %.*s\n", dist + 1, m + idx - dist);
						printf("Rule for first char: %c => %s\n", rp->a, rp->m);
						printf("Rule for last char:  %c => %s\n", r->a, r->m);
					}
					return true;
				}
			}
			rp = rp->next;
		}
	}
	return false;
}

struct m_list {
	char* m;
	struct m_list* next;
};

int process_calmolecule(char* line, char* atoms[], struct rule* rules) {
	struct m_list* m_list = NULL;
	struct m_list* e;
	char* m = molecule_long_to_short_name(line, atoms);
	printf("%s\n", m);
	int lm = strlen(m);

	for (int ii = 0; ii < lm; ++ii) {
		struct rule* r = rules;
		while (r) {
			if (r->a == m[ii]) { // found replacement rule
				char* newm = make_replacement(m, ii, r);
				// check for presence in list
				e = m_list;
				bool is_duplicate = check_duplicate(m, ii, r, rules, false);
				bool found = false;
				while (!found && e) {
					if (!strcmp(newm, e->m)) {
						if (!is_duplicate)
							printf("\ncheck_duplicate missed one:\n%s @ %d\n", m, ii);
						free(newm);
						found = true;
					}
					e = e->next;
				}
				if (!found) { // cons
					if (is_duplicate) {
						printf("\ncheck_duplicate: false positive:\n%s @ %d\n", m, ii);
						check_duplicate(m, ii, r, rules, true);
					}
					e = malloc(sizeof(m_list));
					e->m = newm;
					e->next = m_list;
					m_list = e;
				}
			}
			r = r->next;
		}
	}

	// clean up, and count
	int count = 0;
	while (m_list) {
		e = m_list;
		m_list = m_list->next;
		free(e->m);
		free(e);
		++count;
	}
	free(m);
	return count;
}

int main(int argc, char* argv[]) {
	char* atoms[MAX_ATOMS + 1]; // nul terminated char* array with "long" atom names
	char atom[2]; // "long" name
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
	// read calibration modecule
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		count = process_calmolecule(line, atoms, rules);
	}
	printf("%d\n", count);

	free(line);
	// clean up
	while (rules) {
		struct rule* r = rules;
		rules = r->next;
		free(r);
	}
	for (ii = 0; atoms[ii] != NULL; ++ii)
		free(atoms[ii]);
	return 0;
}
