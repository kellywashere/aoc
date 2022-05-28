#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAMES 128

enum condition {
	// bitfield actually:
	LT = 4,
	LE = 6,
	EQ = 2,
	NE = 0,
	GE = 3,
	GT = 1
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

bool read_condition(char** pLine, enum condition* cond) {
	// skips characters not in [!,<,>,=] before word automatically
	char* l = *pLine;
	int c = -1;
	while (*l && *l != '!' && *l != '<' && *l != '>' && *l != '=')
		++l;
	if (*l == '\0')
		return false;
	if (*(l+1) == '=') {
		if (*l == '!')
			c = NE;
		else if (*l == '=')
			c = EQ;
		else if (*l == '<')
			c = LE;
		else if (*l == '>')
			c = GE;
		l += 2;
	}
	else { // single char condition
		if (*l == '<')
			c = LT;
		else if (*l == '>')
			c = GT;
		l += 1;
	}
	if (c == -1)
		return false;
	*cond = c;
	*pLine = l;
	return true;
}

bool read_word(char** pLine, char* word) {
	// skips characters not in [a-z] before word automatically
	char* line = *pLine;
	int len = 0;
	while (*line && !(*line >= 'a' && *line <= 'z'))
		++line;
	while (*line >= 'a' && *line <= 'z') {
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
	bool isneg = false;
	char* line = *pLine;
	while (*line && *line != '-' && !isdigit(*line))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
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

bool check_condition(int a, enum condition cond, int b) {
	if ( (cond & EQ) > 0 && a == b)
		return true;
	if ( (cond & LT) > 0 && a < b)
		return true;
	if ( (cond & GT) > 0 && a > b)
		return true;
	if (cond == NE && a != b)
		return true;
	return false;
}

int process_line(char* l, int* regs, char* names[]) {
	// returns highest reg value after operation
	char word[32];
	int amount, condval;
	read_word(&l, word);
	int dstreg_idx = name_to_idx(word, names);
	read_word(&l, word);
	bool do_inc = !strcmp(word, "inc");
	read_int(&l, &amount);
	amount = do_inc ? amount : -amount; // turn dec into inc
	read_word(&l, word); // 'if'
	read_word(&l, word); // cond register
	int condreg_idx = name_to_idx(word, names);
	enum condition cond = NE;
	read_condition(&l, &cond);
	read_int(&l, &condval);
	if (check_condition(regs[condreg_idx], cond, condval))
		regs[dstreg_idx] += amount;
	// find largest reg value
	int max_val = regs[0];
	for (int ii = 1; names[ii] != NULL; ++ii)
		max_val = regs[ii] > max_val ? regs[ii] : max_val;
	return max_val;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	int regs[MAX_NAMES];
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place
	for (int ii = 0; ii < MAX_NAMES; ++ii)
		regs[ii] = 0;
	int max_val = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		int maxval_now = process_line(line, regs, names);
		max_val = maxval_now > max_val ? maxval_now : max_val;
	}
	free(line);

	printf("%d\n", max_val);

	// clean up
	for (int ii = 0; names[ii] != NULL; ++ii)
		free(names[ii]);
	return 0;
}
