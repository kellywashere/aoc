#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAMES 1024


// struct representing one line from input
struct bag_contents {
	int bag_sz;  // nr of bag types contained in this bag
	int bag_id[8];
	int amount[8];
};

int read_int(char** pLine) {
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
	*pLine = line;
	return num;
}

int name_to_idx(char* name, char* names[]) {
	// if name not in names, adds it
	int ii;
	for (ii = 0; ii < MAX_NAMES && names[ii] != NULL; ++ii)
		if (!strcmp(names[ii], name))
			return ii;
	if (ii < MAX_NAMES) {
		int l = strlen(name);
		names[ii] = malloc((l + 1) * sizeof(char));
		strcpy(names[ii], name);
		return ii;
	}
	fprintf(stderr, "Ran out of space to store name %s!\n", name);
	return -1;
}

char* find_bagname_and_amount(char* line, char* name, int namesz, int* amount) {
	// returns next position to search from, or NULL when not found
	// copies name into `name`
	// namesz is max len of name including nulterm
	*amount = 0;
	char* lbag = strstr(line, "bag");
	if (lbag) {
		char* lname = lbag - 1;
		int nrspaces = 0;
		while (lname >= line && nrspaces < 2) {
			--lname;
			if (*lname == ' ' || lname == line - 1)
				++nrspaces;
		}
		int n = lbag - lname - 2;
		n = n > namesz - 1 ? namesz - 1 : n;
		strncpy(name, lname + 1, n);
		name[n] = '\0';
		if (*lname == ' ') {
			--lname;
			int pw10 = 1;
			while (lname >= line && isdigit(*lname)) {
				*amount += pw10 * (*lname - '0');
				pw10 *= 10;
				--lname;
			}
		}
	}
	return lbag == NULL ? NULL : lbag + 3;
}

void add_bag(struct bag_contents* b, int id, int amount) {
	b->bag_id[b->bag_sz] = id;
	b->amount[b->bag_sz] = amount;
	++b->bag_sz;
}

bool contains(struct bag_contents* b, int id) {
	for (int ii = 0; ii < b->bag_sz; ++ii)
		if (b->bag_id[ii] == id)
			return true;
	return false;
}

int count_bags(struct bag_contents* bc, int id) {
	int sum = 1; // count this bag, plus all bags inside
	for (int ii = 0; ii < bc[id].bag_sz; ++ii)
		sum += bc[id].amount[ii] * count_bags(bc, bc[id].bag_id[ii]);
	return sum;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place

	char name[80];
	// array that collects data from input
	struct bag_contents bag_contents[MAX_NAMES];
	// init container info
	for (int ii = 0; ii < MAX_NAMES; ++ii)
		bag_contents[ii].bag_sz = 0;
	int max_bag_id = 0;

	// collect info from input
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int amount;
		char* l = line;
		l = find_bagname_and_amount(l, name, 80, &amount);
		int id_container = name_to_idx(name, names);
		max_bag_id = id_container > max_bag_id ? id_container : max_bag_id;
		bag_contents[id_container].bag_sz = 0;
		do {
			l = find_bagname_and_amount(l, name, 80, &amount);
			if (l && strcmp(name, "no other") != 0) {
				int id = name_to_idx(name, names);
				max_bag_id = id > max_bag_id ? id : max_bag_id;
				add_bag(&bag_contents[id_container], id, amount);
			}
		} while (l);
	}

	int shiny_gold_id = name_to_idx("shiny gold", names);
	int nrbags = count_bags(bag_contents, shiny_gold_id);
	printf("%d\n", nrbags - 1); // -1: do not count outer bag

	free(line);
	return 0;
}
