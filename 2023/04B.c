#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_CARDIDS 256

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

void skip_white(char** pLine) {
	char* l = *pLine;
	while (isblank(*l))
		++l;
	*pLine = l;
}

bool read_int(char** pLine, int* x) {
	// skips all chars until a digit is found, then reads nr
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
	*x = num;
	return true;
}

int count_winning_nrs(char* l) {
	bool winning[100] = {false};
	skip_white(&l);
	int nr;
	while (isdigit(*l)) {
		read_int(&l, &nr);
		winning[nr] = true;
		skip_white(&l);
	}
	++l; // skip '|'
	int wins = 0;
	while (read_int(&l, &nr))
		wins += winning[nr] ? 1 : 0;
	return wins;
}

int main(int argc, char* argv[]) {

	// step 1: turn each card into a single nr
	int cardvalue[MAX_CARDIDS];

	char *line = NULL;
	size_t len = 0;
	int nrcards = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			char* l = line;
			while (*l != ':')
				++l;
			++l;
			cardvalue[nrcards++] = count_winning_nrs(l);
		}
	}
	free(line);

	// step 2: process the cards until no new cards won
	int card_amount[MAX_CARDIDS];
	for (int ii = 0; ii < nrcards; ++ii)
		card_amount[ii] = 1; // you start out with 1 of each
	int count = nrcards;
	for (int card = 0; card < nrcards; ++card) {
		for (int ii = card + 1; ii < nrcards && ii <= card + cardvalue[card]; ++ii) {
			card_amount[ii] += card_amount[card];
			count += card_amount[card];
		}
	}

	printf("%d\n", count);
	return 0;
}
