#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

struct hand {
	char cards[5];
	int  bid;
};

#define MAX_HANDS 1024

enum type {
	NONE,
	ONEPAIR,
	TWOPAIR,
	THREEOAK,
	FULLHOUSE,
	FOUROAK,
	FIVEOAK
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
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

int cmp_char(const void* pa, const void* pb) {
	return *(char*)pa - *(char*)pb;
}

enum type get_type(const char* cards) {
	char c[5]; // sorted cards, low to high
	memcpy(c, cards, 5 * sizeof(char));
	qsort(c, 5, sizeof(char), cmp_char);
	//printf("%.5s --> %.5s\n", cards, c);
	if (c[0] == c[4])
		return FIVEOAK;
	if (c[0] == c[3] || c[1] == c[4])
		return FOUROAK;
	if ( (c[0] == c[2] && c[3] != c[2] && c[3] == c[4]) ||
	     (c[0] == c[1] && c[2] != c[1] && c[2] == c[4]) )
		return FULLHOUSE;
	if (c[0] == c[2] || c[1] == c[3] || c[2] == c[4])
		return THREEOAK;
	// now we have no >=3 cards equal
	if ( (c[0] == c[1] && (c[2] == c[3] || c[3] == c[4])) ||
	     (c[1] == c[2] && c[3] == c[4]) )
		return TWOPAIR;
	if (c[0] == c[1] || c[1] == c[2] || c[2] == c[3] || c[3] == c[4])
		return ONEPAIR;
	return NONE;
}

int card2val(char c) {
	char lut[] = "TJQKA";
	return isdigit(c) ? c - '0' : strchr(lut, c) - lut + 10;
}

int cmp_hands(const void* pa, const void* pb) {
	const struct hand* ha = pa;
	const struct hand* hb = pb;
	enum type ta = get_type(ha->cards);
	enum type tb = get_type(hb->cards);
	if (ta != tb)
		return ta - tb;
	for (int ii = 0; ii < 5; ++ii)
		if (ha->cards[ii] != hb->cards[ii])
			return card2val(ha->cards[ii]) - card2val(hb->cards[ii]);
	return 0;
}

int main(int argc, char* argv[]) {
	struct hand hands[MAX_HANDS];
	int nr_hands = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			char* l = line;
			for (int ii = 0; ii < 5; ++ii) {
				hands[nr_hands].cards[ii] = *l;
				++l;
			}
			read_int(&l, &hands[nr_hands].bid);
			++nr_hands;
		}
	}
	qsort(hands, nr_hands, sizeof(struct hand), cmp_hands);

	uint64_t sum = 0;
	for (int ii = 0; ii < nr_hands; ++ii) {
		// printf("%.5s\n", hands[ii].cards);
		sum += (ii + 1) * hands[ii].bid;
	}
	printf("%"PRIu64"\n", sum);

	return 0;
}
