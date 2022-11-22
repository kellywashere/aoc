#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAMES 1024


// struct representing one line from input
struct bag_contents {
	int bag_id_sz;  // nr of bag types contained in this bag
	int bag_id[8];
};

// inverted struct (toward outside, rather than inside)
struct contained_by {
	int bag_id_sz; // nr of bags that can contain this bag
	int bag_id[32];
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

char* find_bagname(char* line, char* name, int namesz) {
	// returns next position to search from, or NULL when not found
	// copies name into `name`
	// namesz is max len of name including nulterm
	char* lbag = strstr(line, "bag");
	if (lbag) {
		char* lname = lbag - 1;
		int nrspaces = 0;
		while (lname >= line && nrspaces < 2) {
			--lname;
			if (*lname == ' ' || lname == line - 1)
				++nrspaces;
		}
		++lname;
		int n = lbag - lname - 1;
		n = n > namesz - 1 ? namesz - 1 : n;
		strncpy(name, lname, n);
		name[n] = '\0';
	}
	return lbag == NULL ? NULL : lbag + 3;
}

void add_bag(struct bag_contents* b, int id) {
	b->bag_id[b->bag_id_sz++] = id;
}

bool contains(struct bag_contents* b, int id) {
	for (int ii = 0; ii < b->bag_id_sz; ++ii)
		if (b->bag_id[ii] == id)
			return true;
	return false;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place

	char name[80];
	// array that collects data from input
	struct bag_contents all_bag_contents[MAX_NAMES];
	// init container info
	for (int ii = 0; ii < MAX_NAMES; ++ii)
		all_bag_contents[ii].bag_id_sz = 0;
	int max_bag_id = 0;

	// collect info from input
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		l = find_bagname(l, name, 80);
		int id_container = name_to_idx(name, names);
		max_bag_id = id_container > max_bag_id ? id_container : max_bag_id;
		all_bag_contents[id_container].bag_id_sz = 0;
		do {
			l = find_bagname(l, name, 80);
			if (l && strcmp(name, "no other") != 0) {
				int id = name_to_idx(name, names);
				max_bag_id = id > max_bag_id ? id : max_bag_id;
				add_bag(&all_bag_contents[id_container], id);
			}
		} while (l);
	}

	// invert info (toward outside, rather than toward inside)
	struct contained_by containers[MAX_NAMES];
	for (int ii = 0; ii < MAX_NAMES; ++ii)
		containers[ii].bag_id_sz = 0;

	for (int id = 0; id <= max_bag_id; ++id) { // bag id that is contained
		for (int container_id = 0; container_id <= max_bag_id; ++container_id) {
			if (contains(&all_bag_contents[container_id], id))
				containers[id].bag_id[containers[id].bag_id_sz++] = container_id;
		}
	}

	bool visited[MAX_NAMES] = {0};
	int stack[MAX_NAMES];
	int stack_ptr = 0;
	int shiny_gold_id = name_to_idx("shiny gold", names);
	stack[stack_ptr++] = shiny_gold_id;
	while (stack_ptr) {
		int id = stack[--stack_ptr];
		visited[id] = true;
		for (int ii = 0; ii < containers[id].bag_id_sz; ++ii) {
			int cont_id = containers[id].bag_id[ii];
			// printf("%d can be in %d\n", id, cont_id);
			if (!visited[cont_id])
				stack[stack_ptr++] = cont_id;
		}
	}
	// count visited
	int count = 0;
	for (int ii = 0; ii <= max_bag_id; ++ii)
		count += visited[ii] ? 1 : 0;
	--count; // do not count shiny gld bag itself
	printf("%d\n", count);

	free(line);
	return 0;
}
