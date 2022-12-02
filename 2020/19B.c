#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

// limited data structure flexibility for ease of coding, based on observation input file

#define MAXRULES 200
// maximum number of OR-ed chains:
#define MAX_SUBRULES_PER_RULE 2
// maximum number of rule ids in one subrule:
#define MAX_RULES_PER_SUBRULE 3

#define MAX_SEQLEN 128 /* prevent endless loop in cycle */

struct subrule {
	char leaf_char; // 'a', 'b', or '\0' if not leaf node
	int  rule_id[MAX_RULES_PER_SUBRULE]; // 0 if no rule[] given
};

struct rule {
	struct subrule subrule[MAX_SUBRULES_PER_RULE];
	int            minlen; // minimum length of string that this rule can match
	int            maxlen; // maximum length of string that this rule can match
};

bool is_subrule(struct subrule* sr) {
	return sr->leaf_char != '\0' || sr->rule_id[0] != 0;
}

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

void read_subrule(char **pLine, struct subrule* sr) {
	char* l = *pLine;
	memset(sr, 0, sizeof(struct subrule)); // init all fields to 0
	while (isspace(*l))
		++l;
	if (*l == '"') {
		sr->leaf_char = *(l+1);
		l += 3;
	}
	else {
		int rule_idx = 0;
		while (rule_idx < MAX_RULES_PER_SUBRULE && isdigit(*l)) {
			while (isdigit(*l)) {
				sr->rule_id[rule_idx] = 10 * sr->rule_id[rule_idx] + *l - '0';
				++l;
			}
			++rule_idx;
			while (isspace(*l))
				++l;
		}
	}
	*pLine = l;
}

void read_rule(char *l, struct rule* r) {
	// id: already skipped
	memset(r, 0, sizeof(struct rule)); // init all fields to 0
	int subrule_idx = 0;
	read_subrule(&l, &r->subrule[subrule_idx++]);
	while (subrule_idx < MAX_SUBRULES_PER_RULE && *l == '|') {
		++l;
		read_subrule(&l, &r->subrule[subrule_idx++]);
	}
}

void print_subrule(struct subrule* sr) {
	if (sr->leaf_char)
		printf(" `%c`", sr->leaf_char);
	else {
		for (int ii = 0; ii < MAX_RULES_PER_SUBRULE && sr->rule_id[ii]; ++ii)
			printf(" %d", sr->rule_id[ii]);
	}
}

void print_rule(struct rule* r) {
	print_subrule(&r->subrule[0]);
	for (int ii = 1; ii < MAX_SUBRULES_PER_RULE && r->subrule[ii].rule_id[0]; ++ii) {
		printf(" |");
		print_subrule(&r->subrule[ii]);
	}
	printf("\n");
}

void print_rules(struct rule* r) {
	for (int ii = 0; ii < MAXRULES; ++ii) {
		if (is_subrule(&r[ii].subrule[0])) {
			printf("%d:", ii);
			print_rule(r + ii);
		}
	}
}

bool match_rule(char* str, int len, struct rule* rules, int id);

bool match_subrule(char* str, int len, struct rule* rules, struct subrule* sr, int sr_startidx) {
	// checks if subrule sr matches str with length exactly `len`
	// sr is interpreted starting from idx sr_startidx (allows recursive algo)

	// base case:
	if (len < 0)
		return false;
	// base case: subrule empty
	if (sr_startidx >= MAX_RULES_PER_SUBRULE || !sr->rule_id[sr_startidx])
		return len == 0;
	// possible speed-up: check lengths achievable vs len
	int minl = 0;
	int maxl = 0;
	for (int idx = sr_startidx; idx < MAX_RULES_PER_SUBRULE && sr->rule_id[idx]; ++idx) {
		minl += rules[sr->rule_id[idx]].minlen;
		maxl += rules[sr->rule_id[idx]].maxlen;
	}
	if (len < minl || len > maxl)
		return false;

	struct rule* r = &rules[sr->rule_id[sr_startidx]];
	for (int l = r->minlen; l <= r->maxlen && l <= len; ++l) { // try to match all possible lengths of first rule
		if (match_rule(str, l, rules, sr->rule_id[sr_startidx]) && match_subrule(str + l, len - l, rules, sr, sr_startidx + 1))
			return true;
	}
	return false;
}

bool match_rule(char* str, int len, struct rule* rules, int id) {
	// printf("match_rule(%s, len=%d, ..., rule#: %d)\n", str, len, id);
	// returns true when first len chars of str can be matched exactly by rule #id
	struct rule* rule = &rules[id];
	if (len < rule->minlen || len > rule->maxlen)
		return false;

	// base case: char match
	if (rule->subrule[0].leaf_char)
		return (*str == rule->subrule[0].leaf_char);
	// check all subrules (rules separated by `|`)
	for (int sr_idx = 0; sr_idx < MAX_SUBRULES_PER_RULE; ++sr_idx) {
		struct subrule* subrule = &rule->subrule[sr_idx];
		if (subrule->rule_id[0] == 0)
			return false; // end of subrules, no match found
		if (match_subrule(str, len, rules, subrule, 0))
			return true;
	}
	return false;
}

void rule_minmaxlen(struct rule* rules, int id, int depth) {
	// dynamic programming: rules array remembers min/max when found (memoization in rules itself)
	struct rule* rule = &rules[id];
	// memoized case
	if (rule->minlen > 0 && rule->maxlen > 0)
		return; // already done
	if (depth >= MAX_SEQLEN) {
		rule->minlen = MAX_SEQLEN; // will be corrected later in recursive algo
		rule->maxlen = MAX_SEQLEN;
		return;
	}
	// base case: char match
	if (rule->subrule[0].leaf_char) {
		rule->minlen = 1;
		rule->maxlen = 1;
	}
	else {
		// go over all subrules
		int minsofar = INT_MAX;
		int maxsofar = 0;
		for (int sr_idx = 0; sr_idx < MAX_SUBRULES_PER_RULE && is_subrule(&rule->subrule[sr_idx]); ++sr_idx) {
			// determine min/max for each subrule
			int minl_tot = 0;
			int maxl_tot = 0;
			struct subrule* sr = &rule->subrule[sr_idx];
			// go over all rules in subrule
			for (int r_idx = 0; r_idx < MAX_RULES_PER_SUBRULE; ++r_idx) {
				int rid = sr->rule_id[r_idx];
				if (!rid)
					break;
				struct rule* r = &rules[rid];
				rule_minmaxlen(rules, rid, depth + 1); // get min and max len for rule[rid]
				minl_tot += r->minlen;
				maxl_tot += r->maxlen;
				maxl_tot = maxl_tot > MAX_SEQLEN ? MAX_SEQLEN : maxl_tot;
			}
			minsofar = minl_tot < minsofar ? minl_tot : minsofar;
			maxsofar = maxl_tot > maxsofar ? maxl_tot : maxsofar;
		}
		rule->minlen = minsofar;
		rule->maxlen = maxsofar;
	}
	// printf("minmax for rule %d: %d - %d\n", id, rule->minlen, rule->maxlen);
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct rule rules[MAXRULES];
	memset(rules, 0, MAXRULES * sizeof(struct rule));

	// read rules
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			break;
		char* l = line;
		int id = 0;
		while (isdigit(*l)) {
			id = 10 * id + *l - '0';
			++l;
		}
		++l; // skip colon
		read_rule(l, rules + id);
	}
	// replace rules 8 and 11
	read_rule("42 | 42 8", rules + 8);
	read_rule("42 31 | 42 11 31", rules + 11);
	
	// print_rules(rules);

	// init minlen/maxlen data for algo speed-up
	rule_minmaxlen(rules, 0, 0); // last 0: depth fro cycle detection
	//printf("%d - %d\n", rules[0].minlen, rules[0].maxlen);

	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		int len = 0;
		while (line[len] == 'a' || line[len] == 'b')
			++len;
		line[len] = '\0'; // for easier printing (debug)

		count += match_rule(line, len, rules, 0) ? 1 : 0;
	}
	printf("%d\n", count);

	free(line);
	return 0;
}
