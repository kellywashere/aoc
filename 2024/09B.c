#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include <assert.h>

//Some conditional compile options:
#define DO_MERGE_EMPTY

#define EMPTY -1

#define MAX_BLOCKS 40000

struct block {
	int    id; // or EMPTY
	size_t size;
};

struct drive {
	struct block* blocks;
	size_t        blocks_sz; // nr of items in `blocks`
};

/*
void print_drive(struct drive* drive) {
	uint64_t loc = 0;
	uint64_t sum = 0;
	uint64_t id, size;
	for (int ii = 0; ii < drive->blocks_sz; ++ii) {
		id = drive->blocks[ii].id;
		size = drive->blocks[ii].size;
		if (id == EMPTY)
			printf("%6" PRIu64 ": ----  x %" PRIu64 "\n", loc, size);
		else {
			int64_t thissum = id * (2*loc + size - 1) * size / 2;
			sum += thissum;
			printf("%6" PRIu64 ": %6" PRIu64 " x %" PRIu64 "  CS:%16" PRIu64 "\n", loc, id, size, sum);
		}
		loc += size;
	}
	printf("\n");
}
*/

void fill_drive(struct drive* drive, const char* line) {
	bool is_empty_space = false;
	int id_count = 0;
	while (isdigit(*line)) {
		size_t size = *line - '0';
		int id = is_empty_space ? EMPTY : id_count;
		if (size > 0) {
			drive->blocks[drive->blocks_sz].id = id;
			drive->blocks[drive->blocks_sz].size = size;
			++drive->blocks_sz;
		}
		id_count += is_empty_space ? 0 : 1;

		is_empty_space = !is_empty_space;
		++line;
	}
}

int find_free_space(struct drive* drive, size_t size) {
	for (int ii = 0; ii < drive->blocks_sz; ++ii)
		if (drive->blocks[ii].id == EMPTY && drive->blocks[ii].size >= size)
			return ii;
	return -1;
}

void move_block(struct drive* drive, int from_idx, int to_idx) {
	assert(drive->blocks[from_idx].id != EMPTY);
	assert(drive->blocks[to_idx].id == EMPTY);
	assert(drive->blocks[from_idx].size > 0);
	assert(drive->blocks[to_idx].size >= drive->blocks[from_idx].size);

	size_t empty_size = drive->blocks[to_idx].size;
	size_t block_size = drive->blocks[from_idx].size;
	int id = drive->blocks[from_idx].id;
	// move file id. We do this now because we mess with the indices
	drive->blocks[from_idx].id = EMPTY;
	drive->blocks[to_idx].id = id;
	// TODO: If we merge empties before we do splitting, blocks_sz never gets larger than original val
	if (empty_size > block_size) { // Create empty block after moved file
		// move all block infos after to_idx 1 pos higher
		size_t bytes_to_move = (drive->blocks_sz - to_idx) * sizeof(drive->blocks[0]);
		memmove(&drive->blocks[to_idx + 1], &drive->blocks[to_idx], bytes_to_move);
		++drive->blocks_sz;
		drive->blocks[to_idx + 1].id = EMPTY;
		drive->blocks[to_idx + 1].size = empty_size - block_size;
		drive->blocks[to_idx].size = block_size;
		from_idx += from_idx > to_idx ? 1 : 0;
	}
#ifdef DO_MERGE_EMPTY
	// merge empty blocks  (NOT NECESSARY, WE NEVER DO ANYTHING WITH THIS MEM ANYMORE...)
	int idx = from_idx;
	if (idx > 0 && drive->blocks[idx - 1].id == EMPTY)
		--idx;
	while (idx + 1 < drive->blocks_sz && drive->blocks[idx + 1].id == EMPTY) { // merge with empty block to right
		drive->blocks[idx].size += drive->blocks[idx + 1].size;
		size_t bytes_to_move = (drive->blocks_sz - idx - 2) * sizeof(drive->blocks[0]);
		if (bytes_to_move > 0)
			memmove(&drive->blocks[idx + 1], &drive->blocks[idx + 2], bytes_to_move);
		--drive->blocks_sz;
	}
#endif
}

void compress_drive(struct drive* drive) {
	// find last data block, to get its ID
	int idx = drive->blocks_sz - 1;
	while (idx >= 0 && drive->blocks[idx].id == EMPTY)
		--idx;
	int id_to_move = drive->blocks[idx].id;
	while (idx >= 0) {
		assert(drive->blocks[idx].id == id_to_move);
		// find free space
		int free_idx = find_free_space(drive, drive->blocks[idx].size);
		if (free_idx != -1 && free_idx < idx) { // always move to left!
			move_block(drive, idx, free_idx);
		}

		// find next id. Note: some shift may have occurred due to merging free space
		// but not enough to shift block we are looking for beyond current value of idx
		--id_to_move;
		while (idx >= 0 && drive->blocks[idx].id != id_to_move)
			--idx;
	}
}

uint64_t calc_checksum(struct drive* drive) {
	uint64_t loc = 0;
	uint64_t sum = 0;
	uint64_t size, id;
	for (int ii = 0; ii < drive->blocks_sz; ++ii) {
		id = drive->blocks[ii].id;
		size = drive->blocks[ii].size;
		if (id != EMPTY)
			sum += id * (2*loc + size - 1)*size / 2;
		loc += drive->blocks[ii].size;
	}
	return sum;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	/*
	int drive_size = calc_space(line);
	int* drive = calloc(drive_size, sizeof(int));
	*/
	struct block* blocks = calloc(MAX_BLOCKS, sizeof(struct block));
	struct drive drive = {
		.blocks = blocks,
		.blocks_sz = 0
	};

	fill_drive(&drive, line);

	compress_drive(&drive);

	printf("%" PRIu64 "\n", calc_checksum(&drive));

	free(drive.blocks);
	free(line);
	return 0;
}
