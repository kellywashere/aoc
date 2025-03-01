#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define __USE_GNU /* for using qsort_r */
#include <stdlib.h>

// algo relies on pages being below 100
#define MAX_PAGENR 99

#define MAX_PAGES 64 /* max pages in printout */

#define MAX_NR_RULES 2048

struct rule {
	int before;
	int after;
};

// for 05B, we need single structure for passing to qsort_r
struct rules {
	struct rule* rules;
	size_t       nr_rules;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_int(const char** pLine, int* x) {
	int num = 0;
	const char* line = *pLine;
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

bool read_rule(const char* line, struct rule* rule) {
	int before, after;
	const char* l = line;
	if (!read_int(&l, &before)) return false;
	if (!read_int(&l, &after)) return false;

	int maxval = before > after ? before : after;
	if (maxval > MAX_PAGENR) {
		fprintf(stderr, "Page number %d too large in rules section\n", maxval);
		return false;
	}

	rule->before = before;
	rule->after = after;
	return true;
}

bool is_correctly_ordered(int pages[], size_t pages_sz, struct rules rules) {
	// translate to index format
	int index[MAX_PAGENR + 1] = {0}; // index[p] gives index of page p, 0 means not in list
	for (size_t ii = 0; ii < pages_sz; ++ii)
		index[pages[ii]] = ii + 1; // 1 based cause 0 means it does not appear in list

	// check the rules
	for (size_t idx_rule = 0; idx_rule < rules.nr_rules; ++idx_rule) {
		int idx_before = index[rules.rules[idx_rule].before];
		int idx_after = index[rules.rules[idx_rule].after];
		if (idx_before > 0 && idx_after > 0 && idx_before > idx_after)
			return false;
	}
	return true;
}


// Comparison function for qsort_r, using the given rules
// Possible improvement would be to binsearch for first value, but whatever
int cmp_pages(const void* pa, const void* pb, void* data) {
	struct rules* rules = (struct rules*)data;
	int pageA = *(int*)pa;
	int pageB = *(int*)pb;
	// try to find a rule with both pages mentioned
	for (size_t ii = 0; ii < rules->nr_rules; ++ii) {
		int before = rules->rules[ii].before;
		int after = rules->rules[ii].after;
		if (pageA == before && pageB == after)
			return -1;
		if (pageA == after && pageB == before)
			return 1;
	}
	return 0;
}


int main(int argc, char* argv[]) {
	struct rule rules_list[MAX_NR_RULES];

	struct rules rules;
	rules.rules = &rules_list[0];
	rules.nr_rules = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		if (read_rule(line, &rules.rules[rules.nr_rules]))
			++rules.nr_rules;
	}

	int tot = 0;
	while (getline(&line, &len, stdin) != -1) {
		const char *l = line;
		int pages[MAX_PAGES];
		size_t pages_sz = 0;
		int p;
		while (read_int(&l, &p))
			pages[pages_sz++] = p;

		if (!is_correctly_ordered(pages, pages_sz, rules)) {
			qsort_r(pages, pages_sz, sizeof(pages[0]), cmp_pages, &rules);
			tot += pages[pages_sz/2];
		}
	}

	printf("%d\n", tot);

	free(line);
	return 0;
}
