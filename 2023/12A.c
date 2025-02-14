#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_GROUPS 16
#define MAX_OPENS  16

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

void count_groups(char* str, int groupsize[], int* nr_groups) {
	bool ingroup = false;
	*nr_groups = 0;
	while (*str == '.' || *str == '#') {
		if (!ingroup && *str == '#') {
			ingroup = true;
			groupsize[*nr_groups] = 0;
		}
		else if (ingroup && *str != '#') {
			ingroup = false;
			++(*nr_groups);
		}
		if (*str == '#')
			++groupsize[*nr_groups];
		++str;
	}
	if (ingroup)
		++(*nr_groups);
}

int count_sols(char* line) {
	char* l = line;
	int groupsize_ref[MAX_GROUPS];
	int nr_groups_ref = 0;
	int groupsize[MAX_GROUPS];
	int nr_groups = 0;
	int x;
	// read the state group sizes from the line
	while (read_int(&l, &x))
		groupsize_ref[nr_groups_ref++] = x;
	// count and mark ?s
	int idx_open[MAX_OPENS];
	int count_open = 0;
	for (int ii = 0; !isblank(line[ii]); ++ii) {
		if (line[ii] == '?')
			idx_open[count_open++] = ii;
	};
	// calc 2^count_open;
	int pw2 = 1;
	for (int ii = 0; ii < count_open; ++ii)
		pw2 <<= 1;
	// brute force solution trying and counting
	int count = 0;
	for (int n = 0; n < pw2; ++n) {
		// fill in according to n
		for (int ii = 0; ii < count_open; ++ii) {
			int b = (n >> ii) & 1;
			line[idx_open[ii]] = b ? '#' : '.';
		}
		count_groups(line, groupsize, &nr_groups);
		bool issol = nr_groups == nr_groups_ref;
		for (int jj = 0; issol && jj < nr_groups; ++jj)
			issol = issol && groupsize[jj] == groupsize_ref[jj];
		count += issol ? 1 : 0;
	}
	return count;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;

	while (getline(&line, &len, stdin) != -1) {
		sum += count_sols(line);
	}
	free(line);
	printf("%d\n", sum);

	return 0;
}
