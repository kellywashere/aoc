#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int count_vowels(const char* str) {
	int count = 0;
	for (; *str; ++str) {
		if (strchr("aeiou", *str) != NULL)
			++count;
	}
	return count;
}

bool has_double_letter(const char* str) {
	char cprev = *(str++);
	for (; *str; ++str) {
		if (*str == cprev)
			return true;
		cprev = *str;
	}
	return false;
}

bool has_forbidden_seq(const char* str) {
	return ((strstr(str, "ab") != NULL) || 
			(strstr(str, "cd") != NULL) ||
			(strstr(str, "pq") != NULL) ||
			(strstr(str, "xy") != NULL) );
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		bool nice = count_vowels(line) >= 3;
		nice = nice && has_double_letter(line);
		nice = nice && !has_forbidden_seq(line);
		if (nice)
			++count;
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
