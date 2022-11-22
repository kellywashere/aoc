#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define NR_FIELDS 8

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

void process_line(char* l, int* fieldcount, bool* has_cid) {
	// line is already checked to be not empty
	do {
		*has_cid = strncmp(l, "cid", 3) == 0 ? true : *has_cid;
		++(*fieldcount);
		l = next_word(l);
	} while (*l);
}

bool is_valid(int fieldcount, bool has_cid) {
	return (fieldcount == NR_FIELDS || (fieldcount == NR_FIELDS - 1 && !has_cid));
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int fieldcount = 0;
	bool has_cid = false;
	int nr_valid = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line)) {
			nr_valid += is_valid(fieldcount, has_cid) ? 1 : 0;
			fieldcount = 0;
			has_cid = false;
		}
		else
			process_line(line, &fieldcount, &has_cid);
	}
	nr_valid += is_valid(fieldcount, has_cid) ? 1 : 0;
	free(line);

	printf("%d\n", nr_valid);
	return 0;
}
