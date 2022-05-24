#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool val2is_trap[] = {false, true, false, true, true, false, true, false};

int count_row_safetiles(bool* row, int width) {
	int count = 0;
	for (int ii = 1; ii <= width; ++ii)
		count += row[ii] ? 0 : 1;
	return count;
}

void generate_next_row(bool* currow, bool* nextrow, int width) {
	int ii = 0;
	nextrow[ii++] = false;
	int val = (currow[ii] ? 1 : 0);
	for ( ; ii <= width; ++ii) {
		val = ((val & 0x03) << 1) + (currow[ii + 1] ? 1 : 0);
		nextrow[ii] = val2is_trap[val];
	}
	nextrow[ii] = false;
}

int count_all_safetiles(char* line, int rows) {
	int count = 0;
	// count floor width
	int width = 0;
	while (line[width] == '.' || line[width] == '^')
		++width;
	bool* currow  = calloc(2 * width, sizeof(bool));
	bool* nextrow = calloc(2 * width, sizeof(bool));
	// fill currow according to line
	int ii = 0;
	currow[ii++] = false;
	for ( ; ii <= width; ++ii)
		currow[ii] = line[ii - 1] == '^';
	currow[ii] = false;
	
	count += count_row_safetiles(currow, width);
	for (int steps = 0; steps < rows - 1; ++steps) {
		generate_next_row(currow, nextrow, width);
		// swap buffers
		bool* t = nextrow;
		nextrow = currow;
		currow = t;
		count += count_row_safetiles(currow, width);
	}
	return count;
}

int main(int argc, char* argv[]) {
	int rows = 40;
	if (argc > 1)
		rows = atoi(argv[1]);
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	int count = count_all_safetiles(line, rows);
	printf("%d\n", count);
	free(line);
	return 0;
}
