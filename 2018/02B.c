#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_IDS 1024

int idlen(char* l) {
	char* ll = l;
	while (*ll && !isspace(*ll))
		++ll;
	return (int)(ll - l);
}

bool differby1(char* l1, char* l2, int* idx) {
	int len = idlen(l1);
	if (idlen(l2) != len)
		return false;
	int count_diffs = 0;
	for (int ii = 0; ii < len; ++ii) {
		if (l1[ii] != l2[ii]) {
			++count_diffs;
			*idx = ii;
		}
	}
	return count_diffs == 1;
}

int main(int argc, char* argv[]) {
	char* ids[MAX_IDS];
	int nr_ids = 0;
	char answer[80];

	char* line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		ids[nr_ids++] = line;
		line = NULL; // next getline will allocate mem again
	}

	for (int ii = 0; ii < nr_ids - 1; ++ii) {
		for (int jj = ii + 1; jj < nr_ids; ++jj) {
			int idx;
			if (differby1(ids[ii], ids[jj], &idx)) {
				int len = idlen(ids[ii]);
				if (idx > 0)
					memcpy(answer, ids[ii], idx);
				if (idx < idlen(ids[ii]))
					memcpy(answer + idx, ids[ii] + idx + 1, len - idx - 1);
				answer[len - 1] = '\0';
			}
		}
	}
	printf("%s\n", answer);

	for (int ii = 0; ii < nr_ids; ++ii)
		free(ids[ii]);
	return 0;
}
