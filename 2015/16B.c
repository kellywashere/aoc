#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct property {
	char* key;
	int   value;
	struct property* next; // linked list: too small for hash map
};

struct property* find_property(struct property* pl, char* key) {
	while (pl) {
		if (!strcmp(pl->key, key))
			break;
		pl = pl->next;
	}
	return pl;
}

struct property* set_property(struct property* pl, char* key, int value) {
	struct property* p = find_property(pl, key);
	if (!p) {
		p = malloc(sizeof(struct property));
		int l = strlen(key) + 1;
		p->key = malloc(l * sizeof(char));
		memcpy(p->key, key, l * sizeof(char));
		p->next = pl;
		pl = p;
	}
	p->value = value;
	return pl;
}

void destroy_property_list(struct property* pl) {
	while (pl) {
		free(pl->key);
		struct property* p = pl;
		pl = pl->next;
		free(p);
	}
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

bool process_line(char* line, struct property* pl, int* sue) {
	char* t;
	char* delim = " \t\n:";
	strtok(line, delim); // skip "Sue"
	t = strtok(NULL, delim); // Sue nr
	*sue = atoi(t);
	t = strtok(NULL, delim); // first prop name
	bool candidate = true;
	while (t) {
		int val = atoi(strtok(NULL, delim));
		// printf("sue #%d; %s: %d\n", sue, t, val);
		struct property* p = find_property(pl, t);
		if (p) {
			if (!strcmp(t, "cats") || !strcmp(t, "trees"))
				candidate = candidate && p->value < val;
			else if (!strcmp(t, "pomeranians") || !strcmp(t, "goldfish"))
				candidate = candidate && p->value > val;
			else
				candidate = candidate && p->value == val;
		}
		t = strtok(NULL, delim); // next prop name
	}
	return candidate;
}

int main(int argc, char* argv[]) {
	struct property* pl = NULL;
	pl = set_property(pl, "children", 3);
	pl = set_property(pl, "cats", 7);
	pl = set_property(pl, "samoyeds", 2);
	pl = set_property(pl, "pomeranians", 3);
	pl = set_property(pl, "akitas", 0);
	pl = set_property(pl, "vizslas", 0);
	pl = set_property(pl, "goldfish", 5);
	pl = set_property(pl, "trees", 3);
	pl = set_property(pl, "cars", 2);
	pl = set_property(pl, "perfumes", 1);

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		int sue = 0;
		if (process_line(line, pl, &sue))
			printf("Candidate: Sue #%d\n", sue);
	}
	free(line);

	return 0;
}
