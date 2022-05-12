#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOTS    250
#define MAX_OUTPUTS 50

struct target {
	bool is_bot;
	int  nr;
};

struct bot {
	bool has_instruction; // DEBUG
	int v1; // chip nr 1
	int v2; // chip nr 2
	struct target low;
	struct target high;
};

void give_chip_to_bot(struct bot* bots, int* outputs, int botnr, int val);

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && (*line < '0' || *line > '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

void distribute_2chips(struct bot* bots, int* outputs, int botnr) {
	struct bot* bot = bots + botnr;
	if (bot->has_instruction) {
		int low = bot->v1 < bot->v2 ? bot->v1 : bot->v2;
		int high = bot->v1 < bot->v2 ? bot->v2 : bot->v1;
		bot->v1 = 0;
		bot->v2 = 0;
		if (bot->low.is_bot)
			give_chip_to_bot(bots, outputs, bot->low.nr, low);
		else
			outputs[bot->low.nr] = low;
		if (bot->high.is_bot)
			give_chip_to_bot(bots, outputs, bot->high.nr, high);
		else
			outputs[bot->high.nr] = high;
	}
}

void give_chip_to_bot(struct bot* bots, int* outputs, int botnr, int val) {
	// printf("Bot %d receives %d\n", botnr, val);
	struct bot* bot = bots + botnr;
	if (bot->v1 == 0)
		bot->v1 = val;
	else if (bot->v2 == 0)
		bot->v2 = val;
	else
		fprintf(stderr, "Bot %d now has more than 2 chips!\n", botnr);
	if (bot->v2) { // bot has 2 chips
		if (bot->has_instruction) {
			distribute_2chips(bots, outputs, botnr);
		}
	}
}

int main(int argc, char* argv[]) {
	struct bot bots[MAX_BOTS];
	for (int ii = 0; ii < MAX_BOTS; ++ii) {
		bots[ii].has_instruction = false;
		bots[ii].v1 = 0;
		bots[ii].v2 = 0;
	}
	int outputs[MAX_OUTPUTS];
	for (int ii = 0; ii < MAX_OUTPUTS; ++ii)
		outputs[ii] = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (line[0] == 'v') { // value instruction
			char* l = line;
			int val = read_int(&l);
			int botnr = read_int(&l);
			give_chip_to_bot(bots, outputs, botnr, val);
		}
		else { // bot instruction
			char* l = line;
			int botnr = read_int(&l);
			struct bot* bot = bots + botnr;
			bot->has_instruction = true;
			for (int ii = 0; ii < 4; ++ii) // skip: gives low to
				l = next_word(l);
			bot->low.is_bot = *l == 'b';
			bot->low.nr = read_int(&l);
			for (int ii = 0; ii < 4; ++ii) // skip: and high to
				l = next_word(l);
			bot->high.is_bot = *l == 'b';
			bot->high.nr = read_int(&l);
		}
	}

	bool done = false;
	while (!done) {
		done = true;
		// find bot with 2 chips
		int botnr = 0;
		while (botnr < MAX_BOTS && bots[botnr].v2 == 0)
			++botnr;
		if (botnr < MAX_BOTS) {
			done = false;
			if (bots[botnr].has_instruction)
				distribute_2chips(bots, outputs, botnr);
			else
				fprintf(stderr, "No instruction for bot #%d\n", botnr);
		}
	}
	printf("%d\n", outputs[0] * outputs[1] * outputs[2]);

	free(line);
	return 0;
}
