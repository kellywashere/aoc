#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#define EMPTY -1

int calc_space(char* line) {
	int count = 0;
	while (isdigit(*line)) {
		count += *line - '0';
		++line;
	}
	return count;
}

void fill_drive(int* drive, char* line) {
	bool is_empty_space = false;
	int id_count = 0;
	while (isdigit(*line)) {
		int size = *line - '0';
		int id = is_empty_space ? EMPTY : id_count;
		for (int ii = 0; ii < size; ++ii) {
			*drive = id;
			++drive;
		}
		id_count += is_empty_space ? 0 : 1;
		is_empty_space = !is_empty_space;
		++line;
	}
}

void compress_drive(int* drive, int drive_size) {
	int last_data_idx = drive_size - 1; // points to last filled segment
	while (drive[last_data_idx] == EMPTY)
		--last_data_idx;
	int first_empty_idx = 0;
	while (first_empty_idx < drive_size && drive[first_empty_idx] != EMPTY)
		++first_empty_idx;

	while (first_empty_idx < last_data_idx) {
		int id = drive[last_data_idx];
		drive[last_data_idx--] = EMPTY;
		drive[first_empty_idx++] = id;
		while (drive[last_data_idx] == EMPTY)
			--last_data_idx;
		while (first_empty_idx < drive_size && drive[first_empty_idx] != EMPTY)
			++first_empty_idx;
	}
}

uint64_t calc_checksum(int* drive, int drive_size) {
	uint64_t sum = 0;
	for (int ii = 0; ii < drive_size && drive[ii] != EMPTY; ++ii) {
		sum += ii * drive[ii];
	}
	return sum;
}

void print_drive(int* drive, int drive_size) {
	for (int ii = 0; ii < drive_size; ++ii)
		if (drive[ii] == EMPTY)
			printf(".\n");
		else
			printf("%d\n", drive[ii]);
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	int drive_size = calc_space(line);
	int* drive = calloc(drive_size, sizeof(int));

	fill_drive(drive, line);
	//print_drive(drive, drive_size);

	compress_drive(drive, drive_size);
	//print_drive(drive, drive_size);

	printf("%" PRIu64 "\n", calc_checksum(drive, drive_size));

	free(drive);
	free(line);
	return 0;
}
