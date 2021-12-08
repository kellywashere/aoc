#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int process_line(char* line) {
	int count = 0;
	char* delim = " \t\n";
	// skip everything in front of |
	char* t = strtok(line, delim);
	while (t[0] != '|')
		t = strtok(NULL, delim);
	t = strtok(NULL, delim);
	while (t) {
		int l = strlen(t);
		if (l == 2 || l == 4 || l == 3 || l == 7)
			++count;
		t = strtok(NULL, delim);
	}
	return count;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		count += process_line(line);
	}
	printf("%d\n", count);
	free(line);
	return 0;
}
