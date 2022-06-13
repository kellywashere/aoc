#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct task {
	bool done;
	int  nr_open_prereq;
	bool prereqs[26];
};

int main(int argc, char* argv[]) {
	struct task tasks[26];
	for (int ii = 0; ii < 26; ++ii) {
		tasks[ii].done = false;
		tasks[ii].nr_open_prereq = 0;
		for (int jj = 0; jj < 26; ++jj)
			tasks[ii].prereqs[jj] = false;
	}
	int nr_tasks = 0;

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
	while (nr_open_tasks > 0) {
		// find first open task with 0 prereqs
		int task_nr = 0;
		while (task_nr < nr_tasks && (tasks[task_nr].done || tasks[task_nr].nr_open_prereq > 0))
			++task_nr;
		if (task_nr == nr_tasks) {
			fprintf(stderr, "Could not find task without prerequisites!\n");
			return 1;
		}
		printf("%c", 'A' + task_nr); // do tasks (print it)
		tasks[task_nr].done = true;
		--nr_open_tasks;
		// fulfill prereqs
		for (int ii = 0; ii < nr_tasks; ++ii) {
			if (tasks[ii].prereqs[task_nr]) {
				tasks[ii].prereqs[task_nr] = false;
				--tasks[ii].nr_open_prereq;
			}
		}
	}
	printf("\n");
	return 0;
}
