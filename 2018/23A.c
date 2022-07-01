#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct bot {
	int         x;
	int         y;
	int         z;
	int         r;
	struct bot* next;
};

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

int mandist(struct bot* b1, struct bot* b2) {
	int dx = b1->x > b2->x ? b1->x - b2->x : b2->x - b1->x;
	int dy = b1->y > b2->y ? b1->y - b2->y : b2->y - b1->y;
	int dz = b1->z > b2->z ? b1->z - b2->z : b2->z - b1->z;
	return dx + dy + dz;
}

int main(int argc, char* argv[]) {
	struct bot* bots = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		struct bot* bot = malloc(sizeof(struct bot));
		char* l = line;
		bot->x = read_int(&l);
		bot->y = read_int(&l);
		bot->z = read_int(&l);
		bot->r = read_int(&l);
		bot->next = bots;
		bots = bot;
	}
	free(line);

	// find strongest bot
	int maxr = 0;
	struct bot* strongest = NULL;
	for (struct bot* bot = bots; bot; bot = bot->next) {
		if (bot->r > maxr) {
			maxr = bot->r;
			strongest = bot;
		}
	}
	// count bots in range
	int count = 0;
	for (struct bot* bot = bots; bot; bot = bot->next)
		count += (mandist(bot, strongest) <= strongest->r) ? 1 : 0;
	printf("%d\n", count);


	while (bots) {
		struct bot* b = bots;
		bots = bots->next;
		free(b);
	}
	return 0;
}
