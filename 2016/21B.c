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

void inv_swap_pos(char* pw, int x, int y) {
	char t = pw[x];
	pw[x] = pw[y];
	pw[y] = t;
}

void inv_swap_letters(char* pw, char x, char y) {
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

void rot_left_1(char* pw) {
	char c = pw[0];
	for (int ii = 0; pw[ii + 1]; ++ii) {
		pw[ii] = pw[ii + 1];
		pw[ii + 1] = c;
	}
}

void inv_rot_right(char* pw, int steps) {
	for ( ; steps; --steps)
		rot_left_1(pw);
}

void inv_rot_left(char* pw, int steps) {
	for ( ; steps; --steps)
		rot_right_1(pw);
}

void inv_rot_based_on_idx_x(char* pw, char x) {
	int y = strchr(pw, x) - pw;
	// y = 2*x + 1 (mod l) for x < 4
	// y = 2*x + 2 (mod l) for x >= 4
	// Easiest way to solve this mod eqn: try
	int l = strlen(pw);
	int idx = 0;
	for ( ; idx < 4; ++idx)
		if ((2*idx + 1) % l == y) {
			inv_rot_right(pw, idx + 1);
			return;
		}
	for (idx = 4; idx < l; ++idx)
		if ((2*idx + 2) % l == y) {
			inv_rot_right(pw, idx + 2);
			return;
		}
	printf("No solution found in inv_rot_based_on_idx_x: pw: %s; x: %c\n", pw, x);
}

void inv_reverse_xy(char* pw, int x, int y) {
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

void inv_move(char* pw, int x, int y) {
	move(pw, y, x);
}

void process_line(char* line, char* pw) {
	char *l = line;
	if (!strncmp(line, "swap", 4)) {
		if (strstr(line, "position")) {
			int x = read_int(&l);
			int y = read_int(&l);
			inv_swap_pos(pw, x, y);
		}
		else if ( (l = strstr(line, "letter")) ) {
			inv_swap_letters(pw, line[12], line[26]); // ugly...
		}
	}
	else if (!strncmp(line, "rotate", 6)) {
		if ( (l = strstr(line, "left")) ) {
			inv_rot_left(pw, read_int(&l));
		}
		else if ( (l = strstr(line, "right")) ) {
			inv_rot_right(pw, read_int(&l));
		}
		else if ( (l = strstr(line, "based")) ) {
			inv_rot_based_on_idx_x(pw, line[35]);
		}
	}
	else if (!strncmp(line, "reverse", 7)) {
		int x = read_int(&l);
		int y = read_int(&l);
		inv_reverse_xy(pw, x, y);
	}
	else if (!strncmp(line, "move", 4)) {
		int x = read_int(&l);
		int y = read_int(&l);
		inv_move(pw, x, y);
	}
}

struct instruction {
	char* line;
	struct instruction* next;
};

struct instruction* create_instruction(char* line) {
	struct instruction* instr = malloc(sizeof(struct instruction));
	int l = strlen(line);
	instr->line = malloc(l + 1);
	strcpy(instr->line, line);
	instr->next = NULL;
	return instr;
}

void destroy_instruction(struct instruction* instr) {
	if (instr) {
		free(instr->line);
		free(instr);
	}
}

int main(int argc, char* argv[]) {
	char pw_scr[100];
	strncpy(pw_scr, "fbgdceah", 100);
	if (argc > 1)
		strncpy(pw_scr, argv[1], 100);
	pw_scr[99] = '\0';

	struct instruction* instrlist = NULL;
	struct instruction* instr = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		instr = create_instruction(line);
		instr->next = instrlist;
		instrlist = instr;
	}
	instr = instrlist;
	while (instr) {
		process_line(instr->line, pw_scr);
		instr = instr->next;
	}
	printf("%s\n", pw_scr);
	while (instrlist) {
		instr = instrlist;
		instrlist = instrlist->next;
		destroy_instruction(instr);
	}
	free(line);
	return 0;
}
