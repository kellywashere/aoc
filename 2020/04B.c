#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

char* fields[] = {"byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid", "cid"};
#define NR_FIELDS 8
char* eyecolors[] = {"amb", "blu", "brn", "gry", "grn", "hzl", "oth"};
#define NR_COLORS 7

struct passport {
	bool valid[NR_FIELDS];
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

int count_nr_chars(char* l) {
	int n = 0;
	while (l[n] && !isspace(l[n]))
		++n;
	return n;
}

bool check_year(char* line, int minyr, int maxyr) {
	bool valid = false;
	if (count_nr_chars(line) == 4) {
		int yr = 0;
		for (int ii = 0; ii < 4; ++ii)
			yr = 10 * yr + line[ii] - '0';
		valid = yr >= minyr && yr <= maxyr;
	}
	return valid;
}

bool check_height(char* line) {
	bool valid = false;
	int h = 0;
	while (isdigit(*line)) {
		h = 10 * h + *line - '0';
		++line;
	}
	if (*line == 'c' && *(line + 1) == 'm')
		valid = h >= 150 && h <= 193;
	else if (*line == 'i' && *(line + 1) == 'n')
		valid = h >= 59 && h <= 76;
	return valid;
}

bool check_haircolor(char* line) {
	bool valid = count_nr_chars(line) == 7 && line[0] == '#';
	for (int ii = 1; valid && ii <= 6; ++ii)
		valid = isdigit(line[ii]) || (line[ii] >= 'a' && line[ii] <= 'f');
	return valid;
}

bool check_eyecolor(char* line) {
	bool valid = false;
	if (count_nr_chars(line) == 3)
		for (int ii = 0; !valid && ii < NR_COLORS; ++ii)
			valid = strncmp(line, eyecolors[ii], 3) == 0;
	return valid;
}

bool check_pid(char* line) {
	bool valid = count_nr_chars(line) == 9;
	for (int ii = 0; valid && ii < 9; ++ii)
		valid = isdigit(line[ii]);
	return valid;
}

void check_key_val(char** pLine, struct passport* pp) {
	char* line = *pLine;
	while (*line && isspace(*line))
		++line;
	if (*line) {
		int key_idx = 0;
		for ( ; key_idx < NR_FIELDS; ++key_idx) {
			if (strncmp(line, fields[key_idx], 3) == 0)
				break;
		}
		line += 4; // skip key + :
		//char* fields[] = {"byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid", "cid"};
		if (key_idx == 0)
			pp->valid[key_idx] = check_year(line, 1920, 2002);
		else if (key_idx == 1)
			pp->valid[key_idx] = check_year(line, 2010, 2020);
		else if (key_idx == 2)
			pp->valid[key_idx] = check_year(line, 2020, 2030);
		else if (key_idx == 3)
			pp->valid[key_idx] = check_height(line);
		else if (key_idx == 4)
			pp->valid[key_idx] = check_haircolor(line);
		else if (key_idx == 5)
			pp->valid[key_idx] = check_eyecolor(line);
		else if (key_idx == 6)
			pp->valid[key_idx] = check_pid(line);
	}
	*pLine = next_word(line);
}

void process_line(char* l, struct passport* pp) {
	// line is already checked to be not empty
	do {
		check_key_val(&l, pp);
	} while (*l);
}

bool check_passport(struct passport* pp) {
	bool valid = true;
	for (int ii = 0; ii < NR_FIELDS - 1; ++ii)
		valid = valid && pp->valid[ii];
	return valid;
}

void clear_passport(struct passport* pp) {
	for (int ii = 0; ii < NR_FIELDS; ++ii)
		pp->valid[ii] = false;
}

int main(int argc, char* argv[]) {
	struct passport pp;
	clear_passport(&pp);
	int nr_valid = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line)) {
			nr_valid += check_passport(&pp) ? 1 : 0;
			clear_passport(&pp);
		}
		else
			process_line(line, &pp);
	}
	nr_valid += check_passport(&pp) ? 1 : 0;
	free(line);

	printf("%d\n", nr_valid);
	return 0;
}
