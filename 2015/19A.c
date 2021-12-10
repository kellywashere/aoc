#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
	// if a not in atoms, adds it
	int ii;
	for (ii = 0; atoms[ii] != NULL; ++ii)
		if (!strcmp(atoms[ii], a))
			return ii;
	int l = strlen(a);
	atoms[ii] = malloc((l + 1) * sizeof(char));
	strcpy(atoms[ii], a);
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

void process_calmolecule(char* line, char* atoms[]) {
	char* m = molecule_long_to_short_name(line, atoms);
	printf("%s\n", m);
}

int main(int argc, char* argv[]) {
	char* atoms[MAX_ATOMS + 1]; // nul terminated char* array with "long" atom names
	char atom[2]; // "long" name
	int ii;
	for (ii = 0; ii < MAX_ATOMS + 1; ++ii)
		atoms[ii] = NULL;

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
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		process_calmolecule(line, atoms);
	}

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
