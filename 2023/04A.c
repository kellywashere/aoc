#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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

int main(int argc, char* argv[]) {
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			char* l = line;
			int cardid;
			read_int(&l, &cardid);
			++l; // skip ':'
			bool winning[100] = {false};
			skip_white(&l);
			int nr;
			while (isdigit(*l)) {
				read_int(&l, &nr);
				winning[nr] = true;
				skip_white(&l);
			}
			++l; // skip '|'
			int worth = 1; // will be corrected later
			while (read_int(&l, &nr))
				worth = winning[nr] ? 2 * worth : worth;
			worth = worth == 1 ? 0 : worth / 2;
			sum += worth;
		}
	}
	free(line);

	printf("%d\n", sum);
	return 0;
}
