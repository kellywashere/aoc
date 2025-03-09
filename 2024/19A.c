#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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

bool can_make_rec(char* design, size_t design_len, struct towel* pat, size_t pat_sz, bool* lut) {
	// lut[design_len] false --> we have already seen this design to be impossible
	if (design_len == 0)
		return true;
	if (!lut[design_len])
		return false;
	// TODO: create array of start indices in pat, so we can quickly match first symbol
	for (size_t ii = 0; ii < pat_sz; ++ii) {
		assert(pat[ii].sz > 0);
		if (strncmp(design, pat[ii].pat, pat[ii].sz) == 0 &&
		    can_make_rec(design + pat[ii].sz, design_len - pat[ii].sz, pat, pat_sz, lut))
				return true;
	}
	lut[design_len] = false;
	return false;
}

bool can_make(char* design, size_t design_len, struct towel* pat, size_t pat_sz) {
	// init lut for memoization
	// lut[ii]==false says that if we have ii characters left,
	// we already know that to be impossible
	bool* lut = malloc((design_len + 1) * sizeof(bool));
	for (int ii = 0; ii < design_len + 1; ++ii)
		lut[ii] = true;
	return can_make_rec(design, design_len, pat, pat_sz, lut);
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
	int count = 0;
	char buf[MAX_DESIGN_LEN + 1] = {0};
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		l = line;
		read_pattern(&l, buf, MAX_DESIGN_LEN + 1);
		bool possible = can_make(buf, strlen(buf), towels, towels_sz);
		count += possible  ? 1 : 0;
	}

	printf("%d\n", count);

	free(line);
	return 1;
}
