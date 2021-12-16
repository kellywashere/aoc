#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_ELEMENTS 26

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

struct rule {
	// xy -> z
	char x;
	char y;
	char z;
	struct rule* next; // LL option
};

struct rule* process_rule_line(char* line) {
	struct rule* rule = mymalloc(sizeof(struct rule));
	//printf("%s", line);
	rule->x = *line;
	rule->y = *(line + 1);
	rule->z = *(line + 6);
	return rule;
}

char* process_template_line(char* line) {
	size_t len = 0;
	while (line[len] >= 'A' && line[len] <= 'Z')
		++len;
	char* template = mymalloc((len + 1) * sizeof(char));
	memcpy(template, line, len * sizeof(char));
	template[len] = '\0';
	return template;
}
struct rule* find_rule(struct rule* rules, char x, char y) {
	while (rules) {
		if (rules->x == x && rules->y == y)
			return rules;
		rules = rules->next;
	}
	return NULL;
}

void apply_rules(char** str, struct rule* rules) {
	char* instr = *str;
	size_t lenstr = strlen(instr);
	char* outstr = mymalloc(2 * lenstr); // already has enough room for \0
	size_t instr_idx = 0;
	size_t outstr_idx = 0;

	outstr[outstr_idx++] = instr[instr_idx++];
	for ( ; instr_idx < lenstr; ++instr_idx) {
		struct rule* r = find_rule(rules, instr[instr_idx - 1], instr[instr_idx]);
		if (r)
			outstr[outstr_idx++] = r->z;
		outstr[outstr_idx++] = instr[instr_idx];
	}
	outstr[outstr_idx] = '\0'; // nul term

	free(instr);
	*str = outstr;
}

int main(int argc, char* argv[]) {
	int iters = 10;
	if (argc > 1)
		iters = atoi(argv[1]);
	struct rule* rules = NULL;

	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);
	char* str = process_template_line(line);

	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		struct rule* r = process_rule_line(line);
		r->next = rules;
		rules = r;
	}
	free(line);

	int ii;
	for (ii = 0; ii < iters; ++ii)
		apply_rules(&str, rules);
	//printf("%s\n", str);

	// count
	int counts[MAX_ELEMENTS] = {0};
	for (ii = 0; str[ii]; ++ii)
		++counts[str[ii]-'A'];
	int mincount = 0;
	int maxcount = 0;
	for (ii = 0; ii < MAX_ELEMENTS; ++ii) {
		if (counts[ii] && (!mincount || counts[ii] < mincount))
			mincount = counts[ii];
		if (counts[ii] > maxcount)
			maxcount = counts[ii];
	}
	printf("%d\n", maxcount - mincount);

	// clean up
	free(str);
	while (rules) {
		struct rule* r = rules;
		rules = r->next;
		free(r);
	}
	return 0;
}
