#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define NAME_HASH_BITS       16
#define NAME_HASH_TABLE_SIZE (1<<NAME_HASH_BITS)

struct treenode {
	char             name[12];
	int              weight;

	struct treenode* parent;
	struct treenode* first_child;  // LL
	struct treenode* next_sibling; // allowing to build LL of siblings
};

struct hash_table_entry {
	uint64_t                 hash64; // full 64-bit hash (anti-collision)
	struct treenode*         tree;   // the info we care about
	struct hash_table_entry* next;   // LL in hash table
};

struct trees {
	struct hash_table_entry** hashtable; // hash table that maps name --> tree node
};


uint32_t hash_name(const char* name, uint64_t *full_hash) {
	// Returns full 64 bit hash in full_hash if full_hash != NULL
	// http://www.cse.yorku.ca/~oz/hash.html
	uint64_t hash = 5381;
	for (int ii = 0; name[ii] != '\0'; ++ii)
		hash = (33 * hash) ^ name[ii];
	if (full_hash)
		*full_hash = hash;
	// now collect all bytes of 64 bit nr into single 16 bit nr
	uint32_t hash2 = 0;
	while (hash) {
		hash2 ^= hash & ((1<<NAME_HASH_BITS)-1);
		hash >>= NAME_HASH_BITS;
	}
	return hash2;
}

struct treenode* hash_table_find(struct hash_table_entry** hash_table, char* name) {
	uint64_t hash64;
	uint32_t hash16 = hash_name(name, &hash64);
	struct hash_table_entry* e = hash_table[hash16];
	while (e && e->hash64 != hash64)
		e = e->next;
	if (e)
		return e->tree;
	return NULL;
}

void hash_table_add(struct hash_table_entry** hash_table, struct treenode* tn) {
	if (hash_table_find(hash_table, tn->name)) {
		fprintf(stderr, "hash_table_add: already entry with name `%s`\n", tn->name);
		return;
	}
	uint64_t hash64;
	uint32_t hash16 = hash_name(tn->name, &hash64);
	struct hash_table_entry* e = malloc(sizeof(struct hash_table_entry));
	e->hash64 = hash64;
	e->tree = tn;
	e->next = hash_table[hash16];
	hash_table[hash16] = e;
}

struct trees* create_trees() {
	struct trees* trees = malloc(sizeof(struct trees));
	trees->hashtable = calloc(NAME_HASH_TABLE_SIZE, sizeof(struct hash_table_entry*));
	return trees;
}

void destroy_trees(struct trees* trees) {
	// also destroys all tree nodes
	if (trees) {
		struct hash_table_entry** hashtable = trees->hashtable;
		for (int ii = 0; ii < NAME_HASH_TABLE_SIZE; ++ii) {
			while (hashtable[ii]) {
				struct hash_table_entry* e = hashtable[ii];
				hashtable[ii] = hashtable[ii]->next;
				free(e->tree);
				free(e);
			}
		}
	}
}

struct treenode* trees_find_node(struct trees* trees, char* name) {
	return hash_table_find(trees->hashtable, name);
}

void trees_add_node(struct trees* trees, struct treenode* tn) {
	hash_table_add(trees->hashtable, tn);
}

struct treenode* trees_find_root(struct trees* trees) {
	struct hash_table_entry** hashtable = trees->hashtable;
	struct treenode* root = NULL;
	for (int ii = 0; ii < NAME_HASH_TABLE_SIZE; ++ii) {
		if (hashtable[ii]) {
			root = hashtable[ii]->tree;
			while (root->parent)
				root = root->parent;
			return root;
		}
	}
	return NULL;
}

struct treenode* create_treenode(char* name) {
	struct treenode* tn = malloc(sizeof(struct treenode));
	strcpy(tn->name, name);
	tn->weight = 0;
	tn->parent = NULL;
	tn->first_child = NULL;
	tn->next_sibling = NULL;
	return tn;
}

void tree_connect_node(struct treenode* parent, struct treenode* child) {
	if (parent) {
		struct treenode* children = parent->first_child;
		child->next_sibling = children;
		parent->first_child = child;
	}
	child->parent = parent;
}


char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

bool read_name(char** pLine, char* name) {
	char* line = *pLine;
	int len = 0;
	while (*line && !(*line >= 'a' && *line <= 'z'))
		++line;
	while (*line >= 'a' && *line <= 'z') {
		name[len++] = *line;
		++line;
	}
	if (len > 0)
		name[len] = '\0';
	*pLine = line;
	return len > 0;
}

int tree_get_total_weight(struct treenode* tn) {
	int weight = tn->weight;
	struct treenode* child = tn->first_child;
	while (child) {
		weight += tree_get_total_weight(child);
		child = child->next_sibling;
	}
	return weight;
}

struct treenode* find_unbalanced_child(struct treenode* tn, int* werror) {
	int w[3];
	struct treenode* ch[3];
	int idx = 0;
	*werror = 0;

	struct treenode* child = tn->first_child;
	while (child) {
		ch[idx] = child;
		w[idx] = tree_get_total_weight(child);
		++idx;
		if (idx == 3) {
			if (w[0] != w[1] && w[1] == w[2]) {
				*werror = w[0] - w[1];
				return ch[0];
			}
			if (w[1] != w[0] && w[0] == w[2]) {
				*werror = w[1] - w[0];
				return ch[1];
			}
			if (w[2] != w[0] && w[0] == w[1]) {
				*werror = w[2] - w[0];
				return ch[2];
			}
			w[0] = w[1];
			w[1] = w[2];
			--idx;
		}
		child = child->next_sibling;
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	char name[12];
	struct trees* trees = create_trees();

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		read_name(&l, name);
		struct treenode* tn = trees_find_node(trees, name);
		if (!tn) {
			tn = create_treenode(name);
			trees_add_node(trees, tn);
		}

		tn->weight = read_int(&l);
		while (read_name(&l, name)) {
			struct treenode* child = trees_find_node(trees, name);
			if (!child) {
				child = create_treenode(name);
				trees_add_node(trees, child);
			}
			tree_connect_node(tn, child);
		}
	}
	struct treenode* tn = trees_find_root(trees);
	while (tn) {
		int werror;
		struct treenode* child = find_unbalanced_child(tn, &werror);
		if (!child) {  // no unbalance found: tn itself is culprit
			find_unbalanced_child(tn->parent, &werror);
			printf("%d\n", tn->weight - werror);
			break;
		}
		tn = child;
	}

	free(line);
	destroy_trees(trees);
	return 0;
}
