#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

void swap_pos(char* pw, int x, int y) {
	char t = pw[x];
	pw[x] = pw[y];
	pw[y] = t;
}

void swap_letters(char* pw, char x, char y) {
	for (int ii = 0; pw[ii]; ++ii) {
		if (pw[ii] == x)
			pw[ii] = y;
		else if (pw[ii] == y)
			pw[ii] = x;
	}
}

void rot_right_1(char* pw) {
	char c = pw[0];
	for (int ii = 1; pw[ii]; ++ii) {
		char t = c;
		c = pw[ii];
		pw[ii] = t;
	}
	pw[0] = c;
}

void rot_right(char* pw, int steps) {
	for ( ; steps; --steps)
		rot_right_1(pw);
}

void rot_left_1(char* pw) {
	char c = pw[0];
	for (int ii = 0; pw[ii + 1]; ++ii) {
		pw[ii] = pw[ii + 1];
		pw[ii + 1] = c;
	}
}

void rot_left(char* pw, int steps) {
	for ( ; steps; --steps)
		rot_left_1(pw);
}

void rot_based_on_idx_x(char* pw, char x) {
	int idx = 0;
	while (pw[idx] && pw[idx] != x)
		++idx;
	if (pw[idx])
		rot_right(pw, 1 + idx + (idx >=4 ? 1: 0));
}

void reverse_xy(char* pw, int x, int y) {
	while (y > x) {
		char t = pw[x];
		pw[x] = pw[y];
		pw[y] = t;
		++x;
		--y;
	}
}

void move(char* pw, int x, int y) {
	char t = pw[x];
	while (x < y) {
		pw[x] = pw[x + 1];
		++x;
	}
	while (x > y) {
		pw[x] = pw[x - 1];
		--x;
	}
	pw[y] = t;
}

void process_line(char* line, char* pw) {
	char *l = line;
	if (!strncmp(line, "swap", 4)) {
		if (strstr(line, "position")) {
			int x = read_int(&l);
			int y = read_int(&l);
			swap_pos(pw, x, y);
		}
		else if ( (l = strstr(line, "letter")) ) {
			swap_letters(pw, line[12], line[26]); // ugly...
		}
	}
	else if (!strncmp(line, "rotate", 6)) {
		if ( (l = strstr(line, "left")) ) {
			rot_left(pw, read_int(&l));
		}
		else if ( (l = strstr(line, "right")) ) {
			rot_right(pw, read_int(&l));
		}
		else if ( (l = strstr(line, "based")) ) {
			rot_based_on_idx_x(pw, line[35]);
		}
	}
	else if (!strncmp(line, "reverse", 7)) {
		int x = read_int(&l);
		int y = read_int(&l);
		reverse_xy(pw, x, y);
	}
	else if (!strncmp(line, "move", 4)) {
		int x = read_int(&l);
		int y = read_int(&l);
		move(pw, x, y);
	}
}

int main(int argc, char* argv[]) {
	char pw[100];
	strncpy(pw, "abcdefgh", 100);
	if (argc > 1)
		strncpy(pw, argv[1], 100);
	pw[99] = '\0';

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		process_line(line, pw);
	printf("%s\n", pw);
	free(line);
	return 0;
}
