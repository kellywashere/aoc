#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define GRIDW 50
#define GRIDH  6

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
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

void show_display(bool* display) {
	for (int y = 0; y < GRIDH; ++y) {
		for (int x = 0; x < GRIDW; ++x)
			printf("%c", display[y * GRIDW + x] ? '#' : '.');
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	bool* display = calloc(GRIDW * GRIDH, sizeof(bool));
	bool* newdisp = calloc(GRIDW * GRIDH, sizeof(bool));

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		memcpy(newdisp, display, GRIDW * GRIDH * sizeof(bool));
		if (line[1] == 'e') { // rect cmd
			char* l = next_word(line);
			int w = read_int(&l);
			++l; // skip x
			int h = read_int(&l);
			for (int y = 0; y < h; ++y)
				for (int x = 0; x < w; ++x)
					newdisp[y * GRIDW + x] = true;
		}
		else { // rotate
			char* l = next_word(line);
			if (*l == 'c') { // rotate column
				l = next_word(l) + 2; // move to and skip x= part
				int x = read_int(&l);
				l = next_word(l); // move to 'by'
				l = next_word(l); // skip 'by'
				int by = read_int(&l);
				for (int y = 0; y < GRIDH; ++y) {
					int ynew = (y + by) % GRIDH;
					newdisp[ynew * GRIDW + x] = display[y * GRIDW + x];
				}
			}
			else { // rotate row
				l = next_word(l) + 2; // move to and skip y= part
				int y = read_int(&l);
				l = next_word(l); // move to 'by'
				l = next_word(l); // skip 'by'
				int by = read_int(&l);
				for (int x = 0; x < GRIDW; ++x) {
					int xnew = (x + by) % GRIDW;
					newdisp[y * GRIDW + xnew] = display[y * GRIDW + x];
				}
			}
		}
		// swap old and new
		bool * t = display;
		display = newdisp;
		newdisp = t;
	}
	show_display(display);
	free(display);
	free(newdisp);
	free(line);
	return 0;
}
