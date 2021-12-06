#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char* argv[]) {
	int days_left = 256;
	int ii;
	uint64_t timers[9]; // timers[n] = #fish with timer set to n

	if (argc > 1)
		days_left = strtol(argv[1],  NULL, 10);

	for (ii = 0; ii < 9; ++ii)
		timers[ii] = 0;

	// get 1 line from stdin
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	// process line -> timers
	char* s = line;
	while (*s) {
		uint64_t n = strtol(s, &s, 10);
		if (*s == ',' || *s == '\n')
			++s;
		++timers[n];
	}

	for (; days_left; --days_left) {
		uint64_t t0 = timers[0];
		for (ii = 0; ii < 8; ++ii)
			timers[ii] = timers[ii + 1];
		timers[6] += t0;
		timers[8] = t0;
	}
	uint64_t sum = 0;
	for (ii = 0; ii < 9; ++ii)
		sum += timers[ii];
	printf("%" PRIu64 "\n", sum);

	free(line);
	return 0;
}
