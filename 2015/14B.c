#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_REINDEER 25

struct reindeer {
	int speed;
	int duration;
	int rest;

	int distance;
	int score;
};

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

char* find_digit(char* str) {
	while (*str && (*str < '0' || *str > '9'))
		++str;
	return *str == '\0' ? NULL : str;
}

void process_line(char* line, int* speed, int* duration, int* rest) {
	char* s = find_digit(line);
	*speed = strtol(s, &s, 10);
	s = find_digit(s);
	*duration = strtol(s, &s, 10);
	s = find_digit(s);
	*rest = strtol(s, &s, 10);
}

int calc_distance(struct reindeer* rd, int time) {
	int tcycle = rd->duration + rd->rest;
	int ncycles = time / tcycle;
	int timeleft = time % tcycle;
	timeleft = timeleft > rd->duration ? rd->duration : timeleft;
	return rd->speed * (ncycles * rd->duration + timeleft);
}

int main(int argc, char* argv[]) {
	int time = 2503;
	if (argc > 1)
		time = atoi(argv[1]);
	struct reindeer reindeer[MAX_REINDEER];
	int nr_reindeer = 0;
	char *line = NULL;
	size_t len = 0;
	int speed, duration, rest;
	while (getline(&line, &len, stdin) != -1) {
		if (!empty_line(line)) {
			process_line(line,
					&reindeer[nr_reindeer].speed,
					&reindeer[nr_reindeer].duration,
					&reindeer[nr_reindeer].rest);
			++nr_reindeer;
		}
	}
	free(line);

	for (int ii = 0; ii < nr_reindeer; ++ii)
		reindeer[ii].score = 0;

	for (int t = 1; t <= time; ++t) {
		int max_dist = 0;
		for (int ii = 0; ii < nr_reindeer; ++ii) {
			int dist = calc_distance(&reindeer[ii], t);
			reindeer[ii].distance = dist;
			max_dist = dist > max_dist ? dist : max_dist;
		}
		for (int ii = 0; ii < nr_reindeer; ++ii) {
			if (reindeer[ii].distance == max_dist)
				++reindeer[ii].score;
		}
	}
	int max_score = 0;
	for (int ii = 0; ii < nr_reindeer; ++ii)
		max_score = reindeer[ii].score > max_score ? reindeer[ii].score : max_score;
	printf("%d\n", max_score);
	return 0;
}
