#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

/*
expr   := sum [* sum]*
sum    := term [+ term]*
term   := number | (expr)
number := digit[digit]*
*/
// NO ERROR CHECKING IMPLEMENTED TO KEEP IT SIMPLE

void skip_white(char** pLine) {
	char* l = *pLine;
	while (isspace(*l))
		++l;
	*pLine = l;
}

int64_t eval_expr(char** pLine);

int64_t eval_term(char** pLine) {
	// term   := number | (expr)
	char* l = *pLine;
	int64_t term_val = 0;
	skip_white(&l);
	if (*l == '(') {
		++l;
		term_val = eval_expr(&l);
		++l; // skip `)`
	}
	else {
		while (isdigit(*l)) {
			term_val = 10 * term_val + *l - '0';
			++l;
		}
	}
	skip_white(&l);
	*pLine = l;
	return term_val;
}

int64_t eval_sum(char** pLine) {
	// sum    := term [+ term]*
	char* l = *pLine;
	int64_t sum_val = eval_term(&l);
	while (*l == '+') {
		++l;
		sum_val += eval_term(&l);
	}
	*pLine = l;
	return sum_val;
}

int64_t eval_expr(char** pLine) {
	// expr   := sum [* sum]*
	char* l = *pLine;
	int64_t expr_val = eval_sum(&l);
	while (*l == '*') {
		++l;
		expr_val *= eval_sum(&l);
	}
	*pLine = l;
	return expr_val;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int64_t sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int64_t val = eval_expr(&l);
		//printf("%"PRId64"\n", val);
		sum += val;
	}
	printf("%"PRId64"\n", sum);
	free(line);
	return 0;
}
