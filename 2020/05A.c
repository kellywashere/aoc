#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

int main(int argc, char* argv[]) {
	int maxid = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int id = 0;
		for (int ii = 0; ii < 10; ++ii)
			id = 2 * id + ((line[ii] == 'B' || line[ii] == 'R') ? 1 : 0);
		maxid = id > maxid ? id : maxid;
	}
	free(line);
	printf("%d\n", maxid);
	return 0;
}
