#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool find_bab(char* line, char a, char b) {
	int brackdepth = 0;
	bool done = false;
	for (int ii = 0; !done; ++ii) {
		if (line[ii] >= 'a' && line[ii] <= 'z') {
			if (brackdepth && ii >= 2 &&
					line[ii - 2] == b && line[ii - 1] == a && line[ii] == b)
				return true;
		}
		else if (line[ii] == '[')
			++brackdepth;
		else if (line[ii] == ']')
			--brackdepth;
		else
			done = true;
	}
	return false;
}

int main(int argc, char* argv[]) {
	int count = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (*line != '[' && (*line < 'a' || *line > 'z'))
			continue;
		int brackdepth = 0;
		bool done = false;
		bool ssl = false;
		for (int ii = 0; !done && !ssl; ++ii) {
			if (line[ii] >= 'a' && line[ii] <= 'z') {
				if (brackdepth == 0 && ii >= 2 &&
						line[ii - 2] == line[ii] && line[ii - 1] != line[ii] &&
						isalpha(line[ii - 1]) ) {
					if (find_bab(line, line[ii], line[ii - 1])) {
						ssl = true;
					}
				}
			}
			else if (line[ii] == '[')
				++brackdepth;
			else if (line[ii] == ']')
				--brackdepth;
			else
				done = true;
		}
		if (ssl)
			++count;
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
