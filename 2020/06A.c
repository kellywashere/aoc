#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

void process_line(char* l, bool* answers) {
	while (*l >= 'a' && *l <= 'z') {
		answers[*l - 'a'] = true;
		++l;
	}
}

void clear_answers(bool* answers) {
	for (int ii = 0; ii < 26; ++ii)
		answers[ii] = false;
}

int count_answers(bool* answers) {
	int sum = 0;
	for (int ii = 0; ii < 26; ++ii)
		sum += answers[ii] ? 1 : 0;
	return sum;
}

int main(int argc, char* argv[]) {
	bool answers[26];
	clear_answers(answers);
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line)) {
			sum += count_answers(answers);
			clear_answers(answers);
		}
		else
			process_line(line, answers);
	}
	sum += count_answers(answers);
	free(line);

	printf("%d\n", sum);
	return 0;
}
