#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct task {
	int  duration;
	bool done;
	bool ongoing;
	int  tstart;
	int  worker; // idx of worker working on this task
	// prereq handling:
	int  nr_open_prereq;
	bool prereqs[26];
};

struct worker {
	bool working;
};

#define MAX_NR_WORKERS 26

int main(int argc, char* argv[]) {
	int nr_workers = 5;
	if (argc > 1)
		nr_workers = atoi(argv[1]);
	int durationA = 61;
	if (argc > 2)
		durationA = atoi(argv[2]);

	struct task tasks[26];
	struct worker workers[MAX_NR_WORKERS];

	// init tasks
	for (int ii = 0; ii < 26; ++ii) {
		tasks[ii].duration = durationA + ii;
		tasks[ii].done = false;
		tasks[ii].ongoing = false;
		tasks[ii].nr_open_prereq = 0;
		for (int jj = 0; jj < 26; ++jj)
			tasks[ii].prereqs[jj] = false;
	}
	int nr_tasks = 0;

	// init workers
	for (int ii = 0; ii < nr_workers; ++ii)
		workers[ii].working = false;

	// read input from stdin
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		int prereq_nr = line[5] - 'A';
		int task_nr = line[36] - 'A';
		nr_tasks = (prereq_nr + 1) > nr_tasks ? prereq_nr + 1 : nr_tasks;
		nr_tasks = (task_nr + 1) > nr_tasks ? task_nr + 1 : nr_tasks;
		if (!tasks[task_nr].prereqs[prereq_nr]) {
			tasks[task_nr].prereqs[prereq_nr] = true;
			++tasks[task_nr].nr_open_prereq;
		}
	}
	free(line);

	int nr_open_tasks = nr_tasks;
	int tick = 0;
	while (nr_open_tasks > 0) {
		int task_nr;
		int worker_nr;
		// find tasks that finish this clock tick
		for (task_nr = 0; task_nr < nr_tasks; ++task_nr) {
			if (tasks[task_nr].ongoing && tick - tasks[task_nr].tstart == tasks[task_nr].duration) {
				tasks[task_nr].ongoing = false;
				tasks[task_nr].done = true;
				workers[tasks[task_nr].worker].working = false; // worker is idle now
				--nr_open_tasks;
				// fulfill prereqs
				for (int ii = 0; ii < nr_tasks; ++ii) {
					if (tasks[ii].prereqs[task_nr]) {
						tasks[ii].prereqs[task_nr] = false;
						--tasks[ii].nr_open_prereq;
					}
				}
			}
		}
		// find tasks that can start now
		do {
			task_nr = 0;
			// find first open task with 0 prereqs
			while (task_nr < nr_tasks &&
					(tasks[task_nr].done || tasks[task_nr].ongoing ||
					 tasks[task_nr].nr_open_prereq > 0))
				++task_nr;

			worker_nr = 0;
			if (task_nr < nr_tasks) {
				// find 1st available worker
				while (worker_nr < nr_workers && workers[worker_nr].working)
					++worker_nr;
				if (worker_nr < nr_workers) { // put worker to work
					workers[worker_nr].working = true;
					tasks[task_nr].ongoing = true;
					tasks[task_nr].tstart = tick;
					tasks[task_nr].worker = worker_nr;
				}
			}
		} while (task_nr < nr_tasks && worker_nr < nr_workers); // keep looking for task & worker combos until one runs out
		++tick;
	}
	printf("%d\n", tick - 1);
	return 0;
}
