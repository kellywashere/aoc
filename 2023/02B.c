#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

enum color {
	NOCOL = -1,
	RED = 0,
	GREEN = 1,
	BLUE = 2
};

char* colstr[] = {"red", "green", "blue"};

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

enum color read_color_from_line(char** pLine) {
	enum color c = NOCOL;
	char* l = *pLine;
	while (*l && !isalpha(*l))
		++l;
	if (isalpha(*l)) {
		for (c = RED; c <= BLUE; ++c) {
			int len = strlen(colstr[c]);
			if (strncmp(l, colstr[c], len) == 0) {
				*pLine = l + strlen(colstr[c]);
				return c;
			}
		}
	}
	*pLine = l;
	return NOCOL;
}

bool read_rgb_from_line(char** pLine, int rgb[]) {
	rgb[0] = rgb[1] = rgb[2] = 0;
	bool data_avail = false;
	char* l = *pLine;
	int val;
	for (;;) {
		if (!read_int(&l, &val))
			break;
		data_avail = true;
		enum color c = read_color_from_line(&l);
		if (c != NOCOL)
			rgb[c] = val;
		skip_white(&l);
		if (*l != ',')
			break;
	}
	*pLine = l;
	return data_avail;
}

int main(int argc, char* argv[]) {
	int rgb[3];
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			char* l = line;
			int id = 0;
			read_int(&l, &id);
			// printf("%s", line);
			int max_rgb[3];
			max_rgb[0] = max_rgb[1] = max_rgb[2] = 0;

			while (read_rgb_from_line(&l, rgb)) {
				// printf("%d red, %d green, %d blue\n", rgb[RED], rgb[GREEN], rgb[BLUE]);
				for (enum color c = RED; c <= BLUE; ++c)
					max_rgb[c] = rgb[c] > max_rgb[c] ? rgb[c] : max_rgb[c];
			}
			sum += max_rgb[RED] * max_rgb[GREEN] * max_rgb[BLUE];
		}
	}
	free(line);

	printf("%d\n", sum);
	return 0;
}
