#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	int idx;
	bool alldifferent = false;
	for (idx = 3; !alldifferent && isalpha(line[idx]); ++idx) {
		alldifferent = true;
		for (int ii = idx - 3; alldifferent && ii < idx; ++ii) {
			for (int jj = ii + 1; alldifferent && jj <= idx; ++jj) {
				if (line[ii] == line[jj])
					alldifferent = false;
			}
		}
	}
	free(line);

	printf("%d\n", idx);
	return 0;
}
