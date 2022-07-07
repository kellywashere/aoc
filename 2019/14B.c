#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

// #define DEBUG

#define MAX_NAMES 128

struct ingredient {
	int amount;
	int el_idx;
	struct ingredient* next; // allow LL
};

struct recipe {
	struct ingredient  out;
	struct ingredient* inlist;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

bool read_word(char** pLine, char* word) {
	// skips non-alpha characters before word automatically
	char* line = *pLine;
	int len = 0;
	while (*line && !isalpha(*line))
		++line;
	while (isalpha(*line)) {
		word[len++] = *line;
		++line;
	}
	if (len > 0)
		word[len] = '\0';
	*pLine = line;
	return len > 0;
}

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

bool read_ingredient(char** pLine, int* amount, int* el_idx, char* names[]) {
	// check if next char is nr
	char *l = *pLine;
	while (*l && (isspace(*l) || (*l) == ','))
		++l;
	if (!isdigit(*l))
		return false;
	read_int(&l, amount);
	char elname[16];
	read_word(&l, elname);
	*el_idx = name_to_idx(elname, names);
	*pLine = l;
	return true;
}

void print_recipes(struct recipe* recipes, char* names[]) {
	int ore_idx = name_to_idx("ORE", names);
	for (int ii = 0; names[ii] != NULL; ++ii) {
		if (ii != ore_idx) {
			printf("%d %s <= ", recipes[ii].out.amount, names[ii]);
			for (struct ingredient* in = recipes[ii].inlist; in; in = in->next) {
				printf("%d %s  ", in->amount, names[in->el_idx]);
			}
			printf("\n");
		}
	}
}

int64_t produce_fuel(int64_t fuel_to_make, struct recipe* recipes, int nr_el, int ore_idx, int fuel_idx) {
	int64_t* amount = calloc(nr_el, sizeof(uint64_t)); // amount[el_idx] is how much of that element I have
	amount[fuel_idx] = -fuel_to_make;

	bool neg_amount = true;
	while (neg_amount) {
		for (int ii = 0; ii < nr_el; ++ii) {
			if (ii != ore_idx && amount[ii] < 0) { // I need this element
				int a = recipes[ii].out.amount; // how much I get from running recipe once
				int64_t n = ((-amount[ii]) + (a - 1)) / a; // ceil(-amount[ii]/a) --> run recipe n times
				amount[ii] += n * a;
				// running recipe n times costs ingedients
				for (struct ingredient* in = recipes[ii].inlist; in; in = in->next)
					amount[in->el_idx] -= n * in->amount;
			}
		}
		// check neg amounts
		neg_amount = false;
		for (int ii = 0; !neg_amount && ii < nr_el; ++ii)
			neg_amount = ii != ore_idx && amount[ii] < 0;
	}
	int64_t ore_used = -amount[ore_idx];
	free(amount);
	return ore_used;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place

	struct recipe recipes[MAX_NAMES]; // recipe[el_idx] describes how to make el_idx

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		char* l = line;
		int amount;
		int el_idx;
		// read input ingredients
		struct ingredient* ingredlist = NULL;
		while (read_ingredient(&l, &amount, &el_idx, names)) {
			struct ingredient* ingred = malloc(sizeof(struct ingredient));
			ingred->amount = amount;
			ingred->el_idx = el_idx;
			ingred->next = ingredlist;
			ingredlist = ingred;
		}
		// read output ingredient
		l = strchr(line, '>') + 1;
		read_ingredient(&l, &amount, &el_idx, names);
		recipes[el_idx].out.amount = amount;
		recipes[el_idx].out.el_idx = el_idx; // not really required
		recipes[el_idx].inlist = ingredlist;
	}
	free(line);

	// print_recipes(recipes, names);

	int ore_idx = name_to_idx("ORE", names);
	int fuel_idx = name_to_idx("FUEL", names);
	recipes[ore_idx].inlist = NULL; // we need nothing to create ORE

	// count elements used
	int nr_el = 0;
	for (int ii = 0; names[ii] != NULL; ++ii)
		++nr_el;

	// find search range itself
	int64_t max_ore = 1000000000000;
	int64_t high = 1;
	while (produce_fuel(high, recipes, nr_el, ore_idx, fuel_idx) <= max_ore)
		high *= 2;
	int64_t low = high / 2;

	// binary search in in range low ... high
	// invariant: `low` can be produced, `high` cannot be produced
	while (low < high - 1) {
		int mid = (low + high) / 2;
		int64_t ore_used = produce_fuel(mid, recipes, nr_el, ore_idx, fuel_idx);
		if (ore_used <= max_ore)
			low = mid;
		else
			high = mid;
	}
	printf("%" PRId64 "\n", low);

	// clean up
	for (int ii = 0; names[ii] != NULL; ++ii) {
		free(names[ii]);
		while (recipes[ii].inlist) {
			struct ingredient* in = recipes[ii].inlist;
			recipes[ii].inlist = recipes[ii].inlist->next;
			free(in);
		}
	}
	return 0;
}
