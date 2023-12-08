#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

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

int card2val(char c) {
	char lut[] = "-23456789TQKA";
	return strchr(lut, c) - lut;
}

int cmp_char(const void* pa, const void* pb) {
	return *(char*)pa - *(char*)pb;
}

int cmp_int_rev(const void* pa, const void* pb) {
	return *(int*)pb - *(int*)pa;
}

enum type get_type(const char* cards) {
	//printf("%.5s --> %.5s\n", cards, c);
	// count cards
	int count[13] = {0};
	for (int ii = 0; ii < 5; ++ii)
		++count[card2val(cards[ii])];
	// dirty hack: set joker count to 0 so it ends up at the back of sorted
	int nr_jokers = count[0];
	count[0] = 0;
	qsort(count, 13, sizeof(int), cmp_int_rev); // large to small

	if (count[0] + nr_jokers == 5)
		return FIVEOAK;
	if (count[0] + nr_jokers == 4)
		return FOUROAK;
	// full house
	int fullhouse3 = MIN(3, count[0] + nr_jokers);
	int jokers_left = nr_jokers - (fullhouse3 - count[0]);
	if (fullhouse3 == 3 && (count[1] + jokers_left) == 2)
		return FULLHOUSE;
	if (count[0] + nr_jokers == 3)
		return THREEOAK;
	// two pairs
	int pair1 = MIN(2, count[0] + nr_jokers);
	jokers_left = nr_jokers - (pair1 - count[0]);
	if (pair1 == 2 && (count[1] + jokers_left) == 2)
		return TWOPAIR;
	if (pair1 == 2)
		return ONEPAIR;
	return NONE;
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
	assert('-' < '2'); // for the joker sort to work

	struct hand hands[MAX_HANDS];
	int nr_hands = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			char* l = line;
			for (int ii = 0; ii < 5; ++ii) {
				char c = *l == 'J' ? '-' : *l;
				hands[nr_hands].cards[ii] = c;
				++l;
			}
			read_int(&l, &hands[nr_hands].bid);
			++nr_hands;
		}
	}
	qsort(hands, nr_hands, sizeof(struct hand), cmp_hands);

	uint64_t sum = 0;
	for (int ii = 0; ii < nr_hands; ++ii) {
		sum += (ii + 1) * hands[ii].bid;
	}
	printf("%"PRIu64"\n", sum);

	return 0;
}
