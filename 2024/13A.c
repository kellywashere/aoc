#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

struct machine {
	int xa;
	int ya;
	int xb;
	int yb;
	int xp;
	int yp;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_int(const char** pLine, int* x) {
	int num = 0;
	const char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

bool read_nonempty_line(char** lineptr, size_t* len) {
	while (getline(lineptr, len, stdin) != -1) {
		if (!empty_line(*lineptr))
			return true;
	}
	return false;
}

bool read_machine(struct machine* m) {
	char *line = NULL;
	size_t len = 0;

	if (!read_nonempty_line(&line, &len))
		return false;
	const char *l = line;
	read_int(&l, &m->xa);
	read_int(&l, &m->ya);

	if (!read_nonempty_line(&line, &len)) {
		free(line);
		return false;
	}
	l = line;
	read_int(&l, &m->xb);
	read_int(&l, &m->yb);

	if (!read_nonempty_line(&line, &len)) {
		free(line);
		return false;
	}
	l = line;
	read_int(&l, &m->xp);
	read_int(&l, &m->yp);

	free(line);
	return true;
}

int main(int argc, char* argv[]) {
	struct machine m;

	int tot = 0;
	while (read_machine(&m)) {
		/*
		printf("Button A: X+%d, Y+%d\n", m.xa, m.ya);
		printf("Button B: X+%d, Y+%d\n", m.xb, m.yb);
		printf("Prize: X=%d, Y=%d\n\n", m.xp, m.yp);
		*/
		int den = m.xa * m.yb - m.ya * m.xb;
		if (den == 0) continue;
		int numa = m.xp * m.yb - m.yp * m.xb;
		if (numa % den != 0) continue;
		int numb = m.yp * m.xa - m.xp * m.ya;
		if (numb % den != 0) continue;
		int a = numa / den;
		int b = numb / den;
		tot += 3*a + b;
	}

	printf("%d\n", tot);

	return 0;
}
