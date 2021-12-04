#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* look_and_say(const char* seq) {
	int l = 2 * strlen(seq) + 1;
	char* nextseq = malloc((l + 1) * sizeof(char)); // will be reduced later on
	int idx = 0;

	while (*seq) {
		const char* strt = seq++;
		while (*strt == *seq)
			++seq;
		int grouplen = seq - strt;
		idx += snprintf(nextseq + idx, l - idx, "%d%c", grouplen, *strt);
	}
	nextseq = realloc(nextseq, (idx + 1) * sizeof(char));
	return nextseq;
}

int main(int argc, char* argv[]) {
	char* seq = "1321131112";
	char* nextseq = NULL;
	if (argc > 1)
		seq = argv[1];
	int maxiters = 40;
	if (argc > 2)
		maxiters = atoi(argv[2]);
	for (int ii = 0; ii < maxiters; ++ii) {
		nextseq = look_and_say(seq);
		// printf("%d: %s -> %s\n", ii + 1, seq, nextseq);
		if (ii > 0)
			free(seq);
		seq = nextseq;
	}
	printf("%d\n", strlen(nextseq));
	free(nextseq);
	return 0;
}

