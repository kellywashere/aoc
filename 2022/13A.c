#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_STRLEN 255

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pStr) {
	int n = 0;
	while (isdigit(**pStr)) {
		n = 10 * n + **pStr - '0';
		++(*pStr);
	}
	return n;
}

int compare_lists(char** pStr1, char** pStr2) {
	// returns negative if list1 < list2, positive if list1 > list2, and 0 for equal

	// base case:
	if (isdigit(**pStr1) && isdigit(**pStr2))
		return (read_int(pStr1) - read_int(pStr2)); // on return: *pStr_ point just after nr

	// **pStr_ is either `[` for a proper list, or a digit
	bool is_proper_list1 = **pStr1 == '[';
	bool is_proper_list2 = **pStr2 == '[';
	*pStr1 += is_proper_list1 ? 1 : 0; // for proper list, move past opening bracket
	*pStr2 += is_proper_list2 ? 1 : 0;
	// compare item by item
	int cmp = 0;
	bool list_over1 = **pStr1 == ']';
	bool list_over2 = **pStr2 == ']';
	while (cmp == 0 && !list_over1 && !list_over2) {
		cmp = compare_lists(pStr1, pStr2); // on return: *pStr_ point just after compared items
		*pStr1 += **pStr1 == ',' ? 1 : 0;
		*pStr2 += **pStr2 == ',' ? 1 : 0;
		list_over1 = **pStr1 == ']' || !is_proper_list1;
		list_over2 = **pStr2 == ']' || !is_proper_list2;
	}
	if (cmp == 0) { // at least one list ran out of items to compare
		if (list_over1 && !list_over2)
			cmp = -1;
		else if (list_over2 && !list_over1)
			cmp = 1;
	}
	// move pointers to just after list
	if (is_proper_list1) {
		while (**pStr1 != ']')
			++(*pStr1);
		++(*pStr1);
	}
	if (is_proper_list2) {
		while (**pStr2 != ']')
			++(*pStr2);
		++(*pStr2);
	}
	return cmp;
}

int compare_strings(char* str1, char* str2) {
	char* s0 = str1;
	char* s1 = str2;
	return compare_lists(&s0, &s1);
}

int main(int argc, char* argv[]) {
	char str[2][MAX_STRLEN + 1];
	char *line = NULL;
	size_t len = 0;

	int stridx = 0;
	int pairidx = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		strncpy(str[stridx], line, MAX_STRLEN);
		++stridx;
		if (stridx == 2) {
			stridx = 0;
			++pairidx;
			int cmp = compare_strings(str[0], str[1]);
			//printf("Pair: %d\n%s%s%d\n", pairidx, str[0], str[1], cmp);
			if (cmp < 0)
				sum += pairidx;
		}
	}
	free(line);
	printf("%d\n", sum);

	return 0;
}
