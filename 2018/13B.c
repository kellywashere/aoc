#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct lines { // LL of lines for temp storage
	char*         line;
	struct lines* next;
};

struct cart {
	bool has_crashed;
	int  x;
	int  y;
	int  dir;
	// when encountering +, we have to remember state
	int  intersection_count; // count encountered intersections
};

const char* dirstr = ">^<v";
int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, -1, 0, 1};

void destroy_lines(struct lines* lines) {
	while (lines) {
		struct lines* l = lines;
		lines = lines->next;
		free(l->line);
		free(l);
	}
}

struct lines* lines_add_line_to_front(struct lines* lines, char* l) {
	char* lcpy = malloc((strlen(l) + 1) * sizeof(char));
	strcpy(lcpy, l);
	struct lines* new_line = malloc(sizeof(struct lines));
	new_line->line = lcpy;
	new_line->next = lines;
	lines = new_line;
	return lines;
}

int line_len(char* line) {
	// strlen up to \n
	char* l = line;
	while (*l && *l != '\n')
		++l;
	return (int)(l - line);
}

int line_count_carts(char* line) {
	int count = 0;
	while (*line && *line != '\n') {
		count += (strchr(dirstr, *line) != NULL) ? 1 : 0;
		++line;
	}
	return count;
}

int cmp_carts(const void* a, const void* b) {
	struct cart* ca = (struct cart*)a;
	struct cart* cb = (struct cart*)b;
	/*
	if (ca.has_crashed != cb.has_crashed)
		return 0;
		*/
	if (ca->has_crashed != cb->has_crashed)
		return (ca->has_crashed ? 1 : 0) - (cb->has_crashed ? 1 : 0);
	return ca->y == cb->y ? ca->x - cb->x : ca->y - cb->y;
}

bool cart_crash(struct cart* carts, int nr_carts, int idx) {
	if (carts[idx].has_crashed)
		return false;
	for (int jj = 0; jj < nr_carts; ++jj) {
		if (jj != idx && !carts[jj].has_crashed) {
			if (carts[jj].x == carts[idx].x && carts[jj].y == carts[idx].y) { // crash
				carts[idx].has_crashed = true;
				carts[jj].has_crashed = true;
				return true;
			}
		}
	}
	return false;
}

void cart_change_dir(struct cart* cart, char gridchar) {
	if (gridchar == '/') {
		// old: > ^ < v  0 1 2 3
		// new: ^ > v <  1 0 3 2
		cart->dir ^= 1;
	}
	else if (gridchar == '\\') {
		// old: > ^ < v  0 1 2 3
		// new: v < ^ >  3 2 1 0
		cart->dir ^= 3;
	}
	else if (gridchar == '+') { //rot left, nop, rot right
		int r = cart->intersection_count;
		if (r == 0) { // rot left
			// old: > ^ < v  0 1 2 3
			// new: ^ < v >  1 2 3 0
			cart->dir = (cart->dir + 1) & 3;
		}
		else if (r == 2) { // rot right
			// old: > ^ < v  0 1 2 3
			// new: v > ^ <  3 0 1 2
			cart->dir = (cart->dir + 3) & 3;
		}
		cart->intersection_count = (cart->intersection_count + 1) % 3;
	}
}

int main(int argc, char* argv[]) {
	int gridw = 0;
	int gridh = 0;
	int nr_carts = 0;
	// read in lines from stdin, extract grid size info as we go
	struct lines* lines = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		lines = lines_add_line_to_front(lines, line);
		int len = line_len(line);
		gridw = len > gridw ? len : gridw;
		++gridh;
		nr_carts += line_count_carts(line);
	}
	free(line);

	// transfer info to grid (lines are in reverse order!)
	char* grid = malloc(gridw * gridh * sizeof(char));
	int y = gridh - 1;
	for (struct lines* l = lines; l != NULL; l = l->next) {
		int len = line_len(l->line);
		for (int x = 0; x < gridw; ++x)
			grid[y * gridw + x] = x < len ? l->line[x] : ' ';
		--y;
	}
	destroy_lines(lines); // we no longer need this temp data

	// init carts
	struct cart* carts = malloc(nr_carts * sizeof(struct cart));
	int cartidx = 0;
	for (int y = 0; y < gridh; ++y) {
		for (int x = 0; x < gridw; ++x) {
			char* d = strchr(dirstr, grid[y * gridw + x]);
			if (d != NULL) {
				carts[cartidx].has_crashed = false;
				carts[cartidx].x = x;
				carts[cartidx].y = y;
				carts[cartidx].dir = (int)(d - dirstr);
				carts[cartidx].intersection_count = 0;
				++cartidx;
			}
		}
	}

	// run loop
	int carts_left = nr_carts;
	while (carts_left > 1) {
		qsort(carts, nr_carts, sizeof(struct cart), cmp_carts);
		for (int ii = 0; ii < nr_carts; ++ii) {
			if (!carts[ii].has_crashed) {
				carts[ii].x += dir2dx[carts[ii].dir];
				carts[ii].y += dir2dy[carts[ii].dir];
				if (cart_crash(carts, nr_carts, ii))
					carts_left -= 2;
				else
					cart_change_dir(&carts[ii], grid[carts[ii].y * gridw + carts[ii].x]);
			}
		}
	}
	qsort(carts, nr_carts, sizeof(struct cart), cmp_carts);
	printf("%d,%d\n", carts[0].x, carts[0].y);

	free(grid);
	free(carts);
	return 0;
}
