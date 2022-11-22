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

void process_line(char* l, int* answers) {
	while (*l >= 'a' && *l <= 'z') {
		++answers[*l - 'a'];
		++l;
	}
}

void clear_answers(int* answers) {
	for (int ii = 0; ii < 26; ++ii)
		answers[ii] = 0;
}

int count_answers(int* answers, int groupsize) {
	int sum = 0;
	for (int ii = 0; ii < 26; ++ii)
		sum += answers[ii] == groupsize ? 1 : 0;
	return sum;
}

int main(int argc, char* argv[]) {
	int answers[26];
	clear_answers(answers);
	int sum = 0;
	int groupsize = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line)) {
			sum += count_answers(answers, groupsize);
			clear_answers(answers);
			groupsize = 0;
		}
		else {
			++groupsize;
			process_line(line, answers);
		}
	}
	sum += count_answers(answers, groupsize);
	free(line);

	printf("%d\n", sum);
	return 0;
}
