#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool read_int(char** pLine, int* x) {
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
	*x = num;
	*pLine = line;
	return true;
}

void spin(char* a, int x) {
	char b[16];
	for (int ii = 0; ii < x; ++ii)
		b[ii] = a[16 - x + ii];
	for (int ii = 0; ii < 16 - x; ++ii)
		a[15 - ii] = a[15 - x - ii];
	for (int ii = 0; ii < x; ++ii)
		a[ii] = b[ii];
}

void exchange(char* a, int i1, int i2) {
	char t = a[i1];
	a[i1] = a[i2];
	a[i2] = t;
}

void partner(char* a, char c1, char c2) {
	int i1 = 0;
	int i2 = 0;
	for (int ii = 0; ii < 16; ++ii) {
		i1 = a[ii] == c1 ? ii : i1;
		i2 = a[ii] == c2 ? ii : i2;
	}
	exchange(a, i1, i2);
}

bool do_instruction(char** pLine, char* a) {
	char* l = *pLine;
	while (*l && isspace(*l))
		++l;
	if (*l == '\0')
		return false;
	if (*l == 's') {
		int x;
		read_int(&l, &x);
		spin(a, x);
	}
	else if (*l == 'x') {
		int x1, x2;
		read_int(&l, &x1);
		read_int(&l, &x2);
		exchange(a, x1, x2);
	}
	else if (*l == 'p') {
		char c1 = *(l+1);
		char c2 = *(l+3);
		l += 4;
		partner(a, c1, c2);
	}
	*pLine = l;
	return true;
}

int main(int argc, char* argv[]) {
	char a[16];
	for (int ii = 0; ii < 16; ++ii)
		a[ii] = 'a' + ii;

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	char* l = line;
	while (do_instruction(&l, a)) {
		if (*l == ',')
			++l;
	}
	printf("%.16s\n", a);

	free(line);
	return 0;
}
