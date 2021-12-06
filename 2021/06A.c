#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int days_left = 80;
	int ii;
	int timers[9]; // timers[n] = #fish with timer set to n

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
		int n = strtol(s, &s, 10);
		if (*s == ',' || *s == '\n')
			++s;
		++timers[n];
	}

	for (; days_left; --days_left) {
		int t0 = timers[0];
		for (ii = 0; ii < 8; ++ii)
			timers[ii] = timers[ii + 1];
		timers[6] += t0;
		timers[8] = t0;
	}
	int sum = 0;
	for (ii = 0; ii < 9; ++ii)
		sum += timers[ii];
	printf("%d\n", sum);

	free(line);
	return 0;
}
