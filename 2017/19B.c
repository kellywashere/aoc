#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct lines { // double LL of lines
	char*         line;
	struct lines* prev;
	struct lines* next;
};

void destroy_lines(struct lines* lines) {
	while (lines) {
		struct lines* l = lines;
		lines = lines->next;
		free(l->line);
		free(l);
	}
}

struct lines* lines_add_line(struct lines* lines, char* l) {
	char* lcpy = malloc((strlen(l) + 1) * sizeof(char));
	strcpy(lcpy, l);
	struct lines* new_line = malloc(sizeof(struct lines));
	new_line->line = lcpy;
	new_line->next = NULL;
	new_line->prev = NULL;
	if (lines) {
		struct lines* last = lines;
		while (last->next)
			last = last->next;
		last->next = new_line;
		new_line->prev = last;
	}
	else
		lines = new_line;
	return lines;
}

int main(int argc, char* argv[]) {
	struct lines* grid = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		grid = lines_add_line(grid, line);
	}
	free(line);

	int xpos = 0;
	int dx = 0;
	int dy = 1;
	// find start pos
	struct lines* l = grid;

	while (l->line[xpos] != '|')
		++xpos;
	bool done = false;
	int steps = 0;
	while (!done) {
		// step & find next +
		do {
			if (dy == -1)
				l = l->prev;
			else if (dy == 1)
				l = l->next;
			else if (dx == -1)
				--xpos;
			else
				++xpos;
			++steps;
		} while (l != NULL && xpos >= 0 && l->line[xpos] != '\0' && l->line[xpos] != ' ' && l->line[xpos] != '+');
		// find new dir
		if (l != NULL && xpos >= 0 && l->line[xpos] == '+') {
			if (dx == 0) {
				dy = 0;
				if (xpos > 0 && l->line[xpos - 1] != ' ')
					dx = -1;
				else
					dx = 1;
			}
			else {
				dx = 0;
				if (l->prev != NULL && l->prev->line[xpos] != ' ')
					dy = -1;
				else
					dy = 1;
			}
		}
		else
			done = true;
	}
	printf("%d\n", steps);

	destroy_lines(grid);
	return 0;
}
