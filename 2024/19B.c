#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

#define MAX_NR_TOWELS 1024
#define MAX_TOWEL_LEN 15
#define MAX_DESIGN_LEN 80

struct towel {
	char   pat[MAX_TOWEL_LEN + 1]; // 0-terminated for string functions
	size_t sz;
};

const char* valid_chars = "wubrg"; // translated to 01234

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_pattern(const char** pLine, char* buf, size_t buf_sz) {
	const char* line = *pLine;
	while (*line && strchr(valid_chars, *line) == NULL)
		++line;
	if (!*line || strchr(valid_chars, *line) == NULL)
		return false;

	size_t sz = 0;
	const char* p = strchr(valid_chars, *line);
	while (sz < buf_sz && p) {
		buf[sz++] = (p - valid_chars) + '0';
		++line;
		p = strchr(valid_chars, *line);
	}
	if (sz == buf_sz)
		return false;
	buf[sz] = '\0';
	*pLine = line;
	return true;
}

int cmp_towels(const void* pa, const void* pb) {
	struct towel* ta = (struct towel*)pa;
	struct towel* tb = (struct towel*)pb;
	return strncmp(ta->pat, tb->pat, MAX_TOWEL_LEN + 1);
}

int64_t count_make_rec(char* design, size_t design_len, struct towel* pat, size_t pat_sz, int64_t* lut) {
	if (design_len == 0)
		return 1;
	if (lut[design_len] != -1)
		return lut[design_len];
	int64_t count = 0;
	for (size_t ii = 0; ii < pat_sz; ++ii) {
		assert(pat[ii].sz > 0);
		if (strncmp(design, pat[ii].pat, pat[ii].sz) == 0) {
		    count += count_make_rec(design + pat[ii].sz, design_len - pat[ii].sz, pat, pat_sz, lut);
		}
	}
	lut[design_len] = count;
	return count;
}

int64_t count_make(char* design, size_t design_len, struct towel* pat, size_t pat_sz) {
	// init lut for memoization
	// lut[ii] says how many ways we can make last ii symbols of design
	// -1 means "we don't know yet"
	int64_t* lut = malloc((design_len + 1) * sizeof(int64_t));
	for (int ii = 0; ii < design_len + 1; ++ii)
		lut[ii] = -1;
	return count_make_rec(design, design_len, pat, pat_sz, lut);
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;

	struct towel towels[MAX_NR_TOWELS];
	size_t towels_sz = 0;

	// Read towel patterns
	if (getline(&line, &len, stdin) == -1)
		return 1;

	const char* l = line;
	while (towels_sz < MAX_NR_TOWELS &&
			read_pattern(&l, towels[towels_sz].pat, MAX_TOWEL_LEN + 1)) {
		towels[towels_sz].sz = strlen(towels[towels_sz].pat);
		++towels_sz;
	}
	qsort(towels, towels_sz, sizeof(struct towel), cmp_towels);

	// process designs
	int64_t count = 0;
	char buf[MAX_DESIGN_LEN + 1] = {0};
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		l = line;
		read_pattern(&l, buf, MAX_DESIGN_LEN + 1);
		int64_t this_count = count_make(buf, strlen(buf), towels, towels_sz);
		count += this_count;
	}

	printf("%" PRIi64 "\n", count);

	free(line);
	return 0;
}
