#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

char* next_word(char* line) {
	// if line starts with word --> jump to beginning next word
	// if line starts with whitespace --> jump to beginning first word
	// returns 0 if no such word found
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return *line == '\0' ? NULL : line;
}

bool words_are_equal(char* word1, char* word2) {
	while (isalpha(*word1) && isalpha(*word2) && *word1 == *word2) {
		++word1;
		++word2;
	}
	return !isalpha(*word1) && !isalpha(*word2);
}

bool is_valid(char* line) {
	char* word1 = line;
	do {
		char* word2 = word1;
		while ( (word2 = next_word(word2)) ) {
			if (words_are_equal(word1, word2))
				return false;
		}
		word1 = next_word(word1);
	} while (word1);
	return true;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1)
		count += is_valid(line) ? 1 : 0;
	printf("%d\n", count);
	free(line);
	return 0;
}
