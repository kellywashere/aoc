#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool ids[1024] = {0}; // 10-bit nrs

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int id = 0;
		for (int ii = 0; ii < 10; ++ii)
			id = 2 * id + ((line[ii] == 'B' || line[ii] == 'R') ? 1 : 0);
		ids[id] = true;
	}
	free(line);
	// now find the "hole"
	int idx;
	for (idx = 0; !ids[idx]; ++idx) ; // find first
	for (; ids[idx]; ++idx) ;
	printf("%d\n", idx);
	return 0;
}
