#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
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

struct tree {
	struct tree* parent;
	struct tree* children[2]; // if NULL: use val
	int val[2]; // leaf value
};

void print_tree_helper(struct tree* root) {
	printf("[");
	if (root->children[0] == NULL)
		printf("%d", root->val[0]);
	else
		print_tree_helper(root->children[0]);
	printf(",");
	if (root->children[1] == NULL)
		printf("%d", root->val[1]);
	else
		print_tree_helper(root->children[1]);
	printf("]");
}

void print_tree(struct tree* root) {
	print_tree_helper(root);
	printf("\n");
}

void destroy_tree(struct tree* root) {
	for (int ii = 0; ii < 2; ++ii)
		if (root->children[ii])
			destroy_tree(root->children[ii]);
}

struct tree* tree_copy(struct tree* tree) {
	struct tree* cpy = mymalloc(sizeof(struct tree));
	cpy->parent = NULL;
	for (int dir = 0; dir < 2; ++dir) {
		if (tree->children[dir]) {
			cpy->children[dir] = tree_copy(tree->children[dir]);
			cpy->children[dir]->parent = cpy;
		}
		else {
			cpy->children[dir] = NULL;
			cpy->val[dir] = tree->val[dir];
		}
	}
	return cpy;
};

int read_int(char* str, int* idx) {
	char* s = str + *idx;
	int x = strtol(s, &s, 10);
	*idx += s - (str + *idx);
	return x;
}

struct tree* str_to_tree_helper(char* str, int* idx, struct tree* parent) {
	++(*idx); // skip [
	struct tree* tree = mymalloc(sizeof(struct tree));
	tree->parent = parent;
	for (int ii = 0; ii < 2; ++ii) {
		if (isdigit(str[*idx])) {
			tree->val[ii] = read_int(str, idx);
			tree->children[ii] = NULL;
		}
		else
			tree->children[ii] = str_to_tree_helper(str, idx, tree);
		++(*idx); // skip , or ]
	}

	return tree;
}

struct tree* str_to_tree(char* str) {
	int idx = 0;
	return str_to_tree_helper(str, &idx, NULL);
}

void explode_add(struct tree* tree) {
	for (int dir = 0; dir < 2; ++dir) { // dir: 0 left, 1 right
		struct tree* t = tree;
		int val = t->val[dir];
		// up until node is children[1-dir] of parent
		while (t->parent && t != t->parent->children[1-dir])
			t = t->parent;
		if (t->parent) {
			if (t->parent->children[dir] == NULL)
				t->parent->val[dir] += val;
			else {
				// go to children[dir] of parent
				t = t->parent->children[dir];
				// follow 1-dir until value found
				while (t->children[1-dir])
					t = t->children[1-dir];
				t->val[1-dir] += val;
			}
		}
	}
}

bool explode(struct tree* tree, int curdepth) {
	if (curdepth == 4) {
		explode_add(tree);
		if (tree == tree->parent->children[0]) {
			tree->parent->children[0] = NULL;
			tree->parent->val[0] = 0;
		}
		else {
			tree->parent->children[1] = NULL;
			tree->parent->val[1] = 0;
		}
		free(tree);
		return true;
	}
	bool exploded = false;
	if (tree->children[0])
		exploded = explode(tree->children[0], curdepth + 1);
	if (!exploded && tree->children[1])
		exploded = explode(tree->children[1], curdepth + 1);
	return exploded;
}

bool split(struct tree* tree) {
	bool split_done = false;
	for (int dir = 0; !split_done && dir < 2; ++dir) {
		if (tree->children[dir])
			split_done = split(tree->children[dir]);
		else if (tree->val[dir] >= 10) {
			tree->children[dir] = mymalloc(sizeof(struct tree));
			tree->children[dir]->parent = tree;
			tree->children[dir]->children[0] = NULL;
			tree->children[dir]->children[1] = NULL;
			tree->children[dir]->val[0] = tree->val[dir] / 2;
			tree->children[dir]->val[1] = (tree->val[dir] + 1) / 2;
			split_done = true;
		}
	}
	return split_done;
}

void reduce(struct tree* tree) {
	bool reduction_done;
	do {
		reduction_done = explode(tree, 0);
		reduction_done = reduction_done || split(tree);
	} while (reduction_done);
}

int magnitude(struct tree* tree) {
	int m[2];
	for (int dir = 0; dir < 2; ++dir) {
		if (tree->children[dir])
			m[dir] = magnitude(tree->children[dir]);
		else
			m[dir] = tree->val[dir];
	}
	return 3 * m[0] + 2 * m[1];
}

struct tree* add(struct tree* t1, struct tree* t2) {
	struct tree* root = mymalloc(sizeof(struct tree));
	root->parent = NULL;
	t1->parent = root;
	t2->parent = root;
	root->children[0] = t1;
	root->children[1] = t2;
	reduce(root);
	return root;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct tree* numbers[1024];
	int nr_numbers = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		numbers[nr_numbers++] = str_to_tree(line);
	}
	int max_mag = 0;
	for (int ii = 0; ii < nr_numbers; ++ii) {
		for (int jj = 0; jj < nr_numbers; ++jj) {
			if (jj != ii) {
				struct tree* t1 = tree_copy(numbers[ii]);
				struct tree* t2 = tree_copy(numbers[jj]);
				/*
				printf("   ");
				print_tree(t1);
				printf(" + ");
				print_tree(t2);
				printf(" = ");
				*/
				struct tree* sum = add(t1, t2);
				// print_tree(sum);
				int mag = magnitude(sum);
				// printf("Magnitude: %d\n", mag);
				if (mag > max_mag)
					max_mag = mag;
				destroy_tree(sum);
			}
		}
	}
	printf("%d\n", max_mag);
	for (int ii = 0; ii < nr_numbers; ++ii)
		destroy_tree(numbers[ii]);
	free(line);
	return 0;
}
