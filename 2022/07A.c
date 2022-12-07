#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_FILENAME_LEN 40
#define SIZE_LIMIT 100000

struct file {
	char         name[MAX_FILENAME_LEN + 1];
	int          size;
	struct file* parent;
	struct file* child; // first child (dir) or NULL for normal file
	struct file* next;  // next sibling
};

struct file* create_file(struct file* parent, char* name, int size) {
	/*
	if (parent)
		printf("Creating %s in %s\n", name, parent->name);
	else
		printf("Creating %s\n", name);
	*/
	struct file* new = malloc(sizeof(struct file));
	strncpy(new->name, name, MAX_FILENAME_LEN + 1);
	new->name[MAX_FILENAME_LEN] = '\0';
	new->parent = parent;
	new->child = NULL;
	new->size = size;
	if (parent != NULL) {
		new->next = parent->child;
		parent->child = new;
	}
	return new;
}

struct file* change_dir(struct file* f, char* name) {
	if (!strcmp(name, "/")) {
		while (f->parent)
			f = f->parent;
	}
	else {
		f = f->child;
		while (f && strncmp(f->name, name, MAX_FILENAME_LEN) != 0)
			f = f->next;
	}
	return f;
}

struct file* dir_up(struct file* f) {
	return f->parent;
}

void destroy_files(struct file* dir) {
	// destroy all children
	while (dir->child) {
		struct file* c = dir->child;
		dir->child = c->next;
		destroy_files(c);
	}
	// printf("Deleting: %s\n", dir->name);
	free(dir);
}

void print_files(struct file* f, int depth) {
	for (int ii = 0; ii < depth; ++ii)
		printf("  ");
	printf("%s ", f->name);
	if (f->child) {
		printf("(dir) %d\n", f->size);
		struct file* c = f->child;
		while (c) {
			print_files(c, depth + 1);
			c = c->next;
		}
	}
	else
		printf("%d\n", f->size);
}

void calc_sizes(struct file* f) {
	int sum = f->size;
	if (f->child) {
		for (struct file* c = f->child; c; c = c->next) {
			calc_sizes(c);
			sum += c->size;
		}
	}
	f->size = sum;
}

// run calc_sizes first!
int sum_dirsizes_upto_lim(struct file* f) {
	if (!f->child)
		return 0; // normal file
	int sum = f->size <= SIZE_LIMIT ? f->size : 0;
	for (struct file* c = f->child; c; c = c->next)
		sum += sum_dirsizes_upto_lim(c);
	return sum;
}

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

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

void read_name(char** pLine, char* name) {
	char* line = *pLine;
	while (*line && isspace(*line))
		++line;
	int len = 0;
	while (len < MAX_FILENAME_LEN && *line != '\0' && !isspace(*line))
		name[len++] = *(line++);
	name[len] = '\0';
	*pLine = line;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct file* root = create_file(NULL, "/", 0);
	char name[MAX_FILENAME_LEN + 1];
	struct file* cur = root;

	char last_cmd[10];

	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (*l == '$') {
			++l;
			while (*l && isspace(*l))
				++l;
			if (!strncmp(l, "cd ", 3)) {
				l += 3;
				read_name(&l, name);
				if (!strcmp(name, ".."))
					cur = dir_up(cur);
				else
					cur = change_dir(cur, name);
				if (!cur) {
					printf("Error: dir not found: %s\n", name);
					return 1;
				}
				strcpy(last_cmd, "cd"); // not necessary...
			}
			else if (!strncmp(l, "ls", 2)) {
				strcpy(last_cmd, "ls"); // not necessary...
			}
		}
		else {
			if (!strcmp(last_cmd, "ls")) { // unnecessary condition...
				int size;
				if (!strncmp(l, "dir ", 4)) {
					size = 0;
					l += 4;
				}
				else {
					size = read_int(&l);
				}
				read_name(&l, name);
				create_file(cur, name, size);
			}
		}
	}
	free(line);

	calc_sizes(root);
	printf("%d\n", sum_dirsizes_upto_lim(root));
	destroy_files(root);

	return 0;
}
