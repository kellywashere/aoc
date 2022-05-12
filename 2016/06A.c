#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct lettercount {
	int count[26];
};

int main(int argc, char* argv[]) {
	struct lettercount* lc = NULL;
	int msglen = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (*line < 'a' || *line > 'z')
			continue;
		if (msglen == 0) {
			while (line[msglen] >= 'a' && line[msglen] <= 'z')
				++msglen;
			lc = calloc(msglen, sizeof(struct lettercount));
		}
		for (int pos = 0; pos < msglen; ++pos)
			++lc[pos].count[line[pos] - 'a'];
	}
	char* msg = malloc((msglen + 1) * sizeof(char));
	for (int pos = 0; pos < msglen; ++pos) {
		int maxcount = 0;
		for (int ii = 0; ii < 26; ++ii) {
			int cnt = lc[pos].count[ii];
			if (cnt > maxcount) {
				maxcount = cnt;
				msg[pos] = 'a' + ii;
			}
		}
	}
	msg[msglen] = '\0';
	printf("%s\n", msg);

	free(line);
	free(lc);
	free(msg);
	return 0;
}
