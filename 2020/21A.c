#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAMES 256

#define MAX_INGRED  256
#define MAX_ALLERG   16
#define MAX_FOODS    64


/* ****************** Integer set **************** */

struct int_set { // can contain integers 0 .. max_items-1
	int   max_items;
	bool* set; // set[n] is true iff n is in set
};

struct int_set* create_int_set(int max_items) {
	struct int_set* set = malloc(sizeof(struct int_set));
	set->max_items = max_items;
	set->set = calloc(max_items, sizeof(bool));
	return set;
}

void destroy_int_set(struct int_set* set) {
	if (set->set) {
		free(set->set);
		free(set);
	}
}

void print_set(struct int_set* set) {
	printf("(0 .. %d): { ", set->max_items - 1);
	for (int ii = 0; ii < set->max_items; ++ii)
		if (set->set[ii])
			printf("%d ", ii);
	printf("}\n");
}

void int_set_clear(struct int_set* set) {
	memset(set->set, 0, set->max_items * sizeof(bool));
}

void int_set_addall(struct int_set* set) {
	for (int ii = 0; ii < set->max_items; ++ii)
		set->set[ii] = true;
}

void int_set_invert(struct int_set* set) {
	for (int ii = 0; ii < set->max_items; ++ii)
		set->set[ii] = !set->set[ii];
}

void int_set_add(struct int_set* set, int el) {
	if (el < 0 || el >= set->max_items)
		fprintf(stderr, "int_set: out of bounds %d (not in 0 .. %d)\n", el, set->max_items - 1);
	else
		set->set[el] = true;
}

void int_set_remove(struct int_set* set, int el) {
	if (el < 0 || el >= set->max_items)
		fprintf(stderr, "int_set: out of bounds %d (not in 0 .. %d)\n", el, set->max_items - 1);
	else
		set->set[el] = false;
}

bool int_set_contains(struct int_set* set, int el) {
	return el >= 0 && el < set->max_items && set->set[el];
}

struct int_set* int_set_unison(struct int_set* set1, struct int_set* set2) {
	// make set2 the bigger one
	if (set1->max_items < set2->max_items) {
		struct int_set* t = set1;
		set1 = set2;
		set2 = t;
	}
	struct int_set* u = create_int_set(set1->max_items);
	for (int ii = 0; ii < u->max_items; ++ii)
		u->set[ii] = set1->set[ii] || (ii < set2->max_items && set2->set[ii]);
	return u;
}

struct int_set* int_set_intersection(struct int_set* set1, struct int_set* set2) {
	// make set1 the smaller one
	if (set1->max_items > set2->max_items) {
		struct int_set* t = set1;
		set1 = set2;
		set2 = t;
	}
	struct int_set* n = create_int_set(set1->max_items);
	for (int ii = 0; ii < n->max_items; ++ii)
		n->set[ii] = set1->set[ii] && set2->set[ii];
	return n;
}

int int_set_nritems(struct int_set* set) {
	int count = 0;
	for (int ii = 0; ii < set->max_items; ++ii)
		count += set->set[ii] ? 1 : 0;
	return count;
}

/* ****************** Integer set **************** */

struct food {
	struct int_set* ingredients;
	struct int_set* allergens;
};

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

bool read_word(char** pLine, char* name) {
	// treats all non-alpha chars as "whitespace"
	char* l = *pLine;
	while (*l && !isalpha(*l))
		++l;
	if (*l == '\0') {
		*pLine = l;
		return false;
	}
	while (isalpha(*l))
		*(name++) = *(l++);
	*name = '\0';
	*pLine = l;
	return true;
}

int main(int argc, char* argv[]) {
	char* ingredients[MAX_NAMES + 1]; // NULL terminated char* array
	char* allergens[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii) {
		ingredients[ii] = NULL; // put all terminations in place
		allergens[ii] = NULL; // put all terminations in place
	}

	struct food foods[MAX_FOODS];
	int nr_foods = 0;

	char name[80];

	// collect info from input
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;

		struct food* fd = &foods[nr_foods];
		fd->ingredients = create_int_set(MAX_INGRED);
		fd->allergens   = create_int_set(MAX_ALLERG);

		bool seen_contains = false;
		while (read_word(&l, name)) {
			if (!strcmp(name, "contains"))
				seen_contains = true;
			else {
				if (!seen_contains) // ingredient
					int_set_add(fd->ingredients, name_to_idx(name, ingredients));
				else // allergen
					int_set_add(fd->allergens, name_to_idx(name, allergens));
			}
		}
		++nr_foods;
	}

	// count ingredients/allergens
	int nr_ingredients = 0;
	while (nr_ingredients < MAX_NAMES && ingredients[nr_ingredients] != NULL)
		++nr_ingredients;
	int nr_allergens = 0;
	while (nr_allergens < MAX_NAMES && allergens[nr_allergens] != NULL)
		++nr_allergens;

	// Per allergen: find set of ingredients that can contain it
	// Remove that set from set of all ingredients
	// what is left is set of ingredients that cannot contain any allergen
	// equivalent: make unison of all "cannot contain allergen x" sets
	// invert that set
	struct int_set* safe = create_int_set(nr_ingredients); // empty set

	for (int alg = 0; alg < nr_allergens; ++alg) {
		struct int_set* s = create_int_set(nr_ingredients);
		int_set_addall(s); // before we look, allergen can be in any ingredient
		for (int fd = 0; fd < nr_foods; ++fd) {
			if (int_set_contains(foods[fd].allergens, alg)) {
				// s = s AND foods[fd].ingredients (intersection)
				struct int_set* n = int_set_intersection(s, foods[fd].ingredients); // alg has to be in one of these ingredients
				destroy_int_set(s);
				s = n;
			}
		}
		/*
		printf("Allergen %s can be in:\n", allergens[alg]);
		for (int ii = 0; ii < nr_ingredients; ++ii)
			if (int_set_contains(s, ii))
				printf("  %s\n", ingredients[ii]);
		*/
		// safe = safe OR s (unison)
		struct int_set* n = int_set_unison(safe, s);
		destroy_int_set(s);
		destroy_int_set(safe);
		safe = n;
	}
	int_set_invert(safe); // now this is the list of ingredients that cannot contain any allergen

	/*
	printf("Safe ingredients:\n");
	for (int ii = 0; ii < nr_ingredients; ++ii)
		if (int_set_contains(safe, ii))
			printf("  %s\n", ingredients[ii]);
	*/

	// Now, per food item, count how many safe ingredients it has
	int count = 0;
	for (int fd = 0; fd < nr_foods; ++fd) {
		struct int_set* s = int_set_intersection(safe, foods[fd].ingredients);
		count += int_set_nritems(s);
		destroy_int_set(s);
	}
	printf("%d\n", count);

	destroy_int_set(safe);
	for (int ii = 0; ii < nr_foods; ++ii) {
		destroy_int_set(foods[ii].ingredients);
		destroy_int_set(foods[ii].allergens);
	}
	for (int ii = 0; ii < MAX_NAMES && ingredients[ii] != NULL; ++ii)
		free(ingredients[ii]);
	for (int ii = 0; ii < MAX_NAMES && allergens[ii] != NULL; ++ii)
		free(allergens[ii]);
	free(line);
	return 0;
}
