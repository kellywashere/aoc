#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define SIZE (20+2)

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int x[3];
	bool cubes[SIZE * SIZE * SIZE] = {0};
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		for (int ii = 0; ii < 3; ++ii)
			x[ii] = read_int(&l) + 1; // +1 to leave edge aroud shape for fill algo
		cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0]] = true;
	}

	int count = 0;
	bool visited[SIZE * SIZE * SIZE] = {0};
	bool onstack[SIZE * SIZE * SIZE] = {0};
	int stack[SIZE * SIZE * SIZE];
	int stackptr = 0;
	stack[stackptr++] = 0; // 0,0,0 is definitely outside
	onstack[0] = true;
	while (stackptr) {
		int idx = stack[--stackptr];
		onstack[idx] = false;
		if (visited[idx])
			continue;
		visited[idx] = true;
		x[2] = idx / (SIZE * SIZE);
		idx = idx % (SIZE * SIZE);
		x[1] = idx / SIZE;
		x[0] = idx % SIZE;
		// count how many lava cubes are bordering this cube
		count += x[2] > 0        && cubes[(x[2] - 1) * SIZE * SIZE + x[1] * SIZE + x[0]] ? 1 : 0;
		count += x[2] < SIZE - 1 && cubes[(x[2] + 1) * SIZE * SIZE + x[1] * SIZE + x[0]] ? 1 : 0;
		count += x[1] > 0        && cubes[x[2] * SIZE * SIZE + (x[1] - 1) * SIZE + x[0]] ? 1 : 0;
		count += x[1] < SIZE - 1 && cubes[x[2] * SIZE * SIZE + (x[1] + 1) * SIZE + x[0]] ? 1 : 0;
		count += x[0] > 0        && cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0] - 1] ? 1 : 0;
		count += x[0] < SIZE - 1 && cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0] + 1] ? 1 : 0;
		// add unvisited neighbors to stack
		int idx2 = (x[2] - 1) * SIZE * SIZE + x[1] * SIZE + x[0];
		if (x[2] > 0 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
		idx2 = (x[2] + 1) * SIZE * SIZE + x[1] * SIZE + x[0];
		if (x[2] < SIZE - 1 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
		idx2 = x[2] * SIZE * SIZE + (x[1] - 1) * SIZE + x[0];
		if (x[1] > 0 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
		idx2 = x[2] * SIZE * SIZE + (x[1] + 1) * SIZE + x[0];
		if (x[1] < SIZE - 1 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
		idx2 = x[2] * SIZE * SIZE + x[1] * SIZE + x[0] - 1;
		if (x[0] > 0 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
		idx2 = x[2] * SIZE * SIZE + x[1] * SIZE + x[0] + 1;
		if (x[0] < SIZE - 1 && !visited[idx2] && !cubes[idx2] && !onstack[idx2]) {
			stack[stackptr++] = idx2;
			onstack[idx2] = true;
		}
	}
	printf("%d\n", count);

	free(line);
	return 0;
}
