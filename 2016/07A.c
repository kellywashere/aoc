#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

int main(int argc, char* argv[]) {
	int count = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (*line != '[' && (*line < 'a' || *line > 'z'))
			continue;
		int brackdepth = 0;
		bool done = false;
		bool abba_outside = false;
		bool abba_inside = false;
		for (int ii = 0; !done; ++ii) {
			if (line[ii] >= 'a' && line[ii] <= 'z') {
				if (ii >= 3) {
					if (line[ii] == line[ii - 3] &&
							line[ii - 1] == line[ii - 2] &&
							line[ii] != line[ii - 1] &&
							isalpha(line[ii - 1]) ) {
						if (brackdepth) {
							abba_inside = true;
							done = true;
						}
						else
							abba_outside = true;
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
		if (abba_outside && !abba_inside)
			++count;
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
