#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

/*
expr   := term [+,* term]*
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
	//printf("eval_term: %s", *pLine);
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

int64_t eval_expr(char** pLine) {
	// expr   := term [+,* term]*
	//printf("eval_expr: %s", *pLine);
	char* l = *pLine;
	int64_t expr_val = eval_term(&l); // skips whitespace before and after!
	while (*l && *l != ')') {
		if (*l == '+') {
			++l;
			expr_val += eval_term(&l);
		}
		else if (*l == '*') {
			++l;
			expr_val *= eval_term(&l);
		}
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
		sum += eval_expr(&l);
	}
	printf("%"PRId64"\n", sum);
	free(line);
	return 0;
}
