#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define WIDTH  7
#define HEIGHT 128 /* circular! */
#define BLOCKS 1000000000000LL

struct block {
	int row[4]; // each row is a 4-bit bitfield
	int w;
};

struct block blocks[] = {
	{{0xF,0x0,0x0,0x0}, 4}, // -
	{{0x4,0xE,0x4,0x0}, 3}, // +
	{{0xE,0x2,0x2,0x0}, 3}, // J
	{{0x8,0x8,0x8,0x8}, 1}, // I
	{{0xC,0xC,0x0,0x0}, 2}, // o
};

struct grid {
	int   w;
	int*  grid; // each row represented as bitfield

	int64_t maxy;
	int64_t block_cnt; // nr of blocks that have fallen
	int     blockx;
	int64_t blocky;
	int     input_idx;
};

/*
void show_grid(struct grid* g) {
	printf("block %d @ %d,%d\n", g->block_cnt, g->blockx, g->blocky);
	for (int y = g->maxy + 7; y >= 0; --y) {
		printf("|");
		for (int x = 0; x < g->w; ++x) {
			char c = '.';
			int by = y - g->blocky;
			int bx = x - g->blockx;
			if (0 <= by && by < 4 && 0 <= bx && bx < 4) {
				int b = blocks[g->block_cnt].row[by];
				b = (b >> (3 - bx)) & 1;
				c = b ? '@' : c;
			}
			c = c == '.' && ((g->grid[y] >> (g->w - 1 - x)) & 1) == 1 ? '#' : c;
			printf("%c", c);
		}
		printf("|\n");
	}
	printf("+");
	for (int x = 0; x < g->w; ++x)
		printf("-");
	printf("+\n");
}
*/

void spawn_block(struct grid* g) {
	// clear rows above ymax (remove history in circular grid)
	for (int dy = 1; dy <= 7; ++dy)
		g->grid[(g->maxy + dy) % HEIGHT] = 0;
	++g->block_cnt;
	g->blockx = 2;
	g->blocky = g->maxy + 4;
}

void freeze_block(struct grid* g) {
	int idx = g->block_cnt % (sizeof(blocks)/sizeof(blocks[0]));
	for (int dy = 0; dy < 4; ++dy) {
		int b = blocks[idx].row[dy];
		int shl = g->w - 4 - g->blockx;
		b = shl >= 0 ? (b << shl) : (b >> (-shl));
		g->grid[(g->blocky + dy) % HEIGHT] |= b;
		if (b && g->blocky + dy > g->maxy)
			g->maxy = g->blocky + dy;
	}
}

struct grid* create_grid(int w) {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = w;
	g->grid = calloc(w * HEIGHT, sizeof(int));
	g->maxy = -1;
	g->block_cnt = -1;
	g->input_idx = 0;
	spawn_block(g);
	return g;
}

struct grid* grid_copy(struct grid* g) {
	struct grid* cpy = create_grid(g->w);
	memcpy(cpy->grid, g->grid, g->w * HEIGHT * sizeof(int));
	cpy->maxy      = g->maxy;
	cpy->block_cnt = g->block_cnt;
	cpy->blockx    = g->blockx;
	cpy->blocky    = g->blocky;
	cpy->input_idx = g->input_idx;
	return cpy;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool grids_equal(struct grid* g1, struct grid* g2) {
	// compares grid contents, ignoring absolute y values
	int idx1 = g1->block_cnt % (sizeof(blocks)/sizeof(blocks[0]));
	int idx2 = g2->block_cnt % (sizeof(blocks)/sizeof(blocks[0]));
	if (idx1 != idx2) return false;
	if (g1->blockx != g2->blockx) return false;
	int64_t dy1 = (g1->blocky + HEIGHT - g1->maxy) % HEIGHT;
	int64_t dy2 = (g2->blocky + HEIGHT - g2->maxy) % HEIGHT;
	if (dy1 != dy2) return false;
	for (int dy = 0; dy < HEIGHT; ++dy)
		if (g1->grid[(g1->maxy + dy) % HEIGHT] != g2->grid[(g2->maxy + dy) % HEIGHT])
			return false;
	return true;
}

bool check_overlap(struct grid* g) {
	int idx = g->block_cnt % (sizeof(blocks)/sizeof(blocks[0]));
	if ((g->blocky < 0) ||
			(g->blockx < 0) ||
			(g->blockx + blocks[idx].w > g->w))
		return true;
	for (int dy = 0; dy < 4; ++dy) {
		int b = blocks[idx].row[dy];
		int shl = g->w - 4 - g->blockx;
		b = shl >= 0 ? (b << shl) : (b >> (-shl));
		if ( (b & g->grid[(g->blocky + dy) % HEIGHT]) )
			return true;
	}
	return false;
}

bool drop_block(struct grid* g) {
	--g->blocky;
	if (check_overlap(g)) {
		++g->blocky;
		return false;
	}
	return true;
}

bool push_block(struct grid* g, int dx) {
	g->blockx += dx;
	if (check_overlap(g)) {
		g->blockx -= dx;
		return false;
	}
	return true;
}

bool run_input_steps(struct grid* g, char* input, int input_len, int steps) {
	// returns true if last step resulted in new block spawning
	bool rest = false;
	for (int ii = 0; ii < steps; ++ii) {
		int dx = input[g->input_idx] == '<' ? -1 : 1;
		g->input_idx = (g->input_idx + 1) % input_len;
		push_block(g, dx);
		rest = !drop_block(g);
		if (rest) {
			freeze_block(g);
			spawn_block(g);
		}
	}
	return rest;
}

void drop_till_rest(struct grid* g, char* input, int input_len) {
	bool new_spawned = false;
	do {
		new_spawned = run_input_steps(g, input, input_len, 1);
	} while (!new_spawned);
}

void floyd(struct grid* g, char* input, int input_len, int* mu, int* lambda) {
	// tortoise-hare cycle detection
	// https://en.wikipedia.org/wiki/Cycle_detection
	struct grid* tortoise = grid_copy(g);
	run_input_steps(tortoise, input, input_len, input_len); // tortoise = f(x0)

	struct grid* hare = grid_copy(g);
	run_input_steps(hare, input, input_len, 2 * input_len); // hare = f(f(x0))
	while (!grids_equal(tortoise, hare)) {
		run_input_steps(tortoise, input, input_len, input_len); // tortoise = f(tortoise)
		run_input_steps(hare, input, input_len, 2 * input_len); // hare = f(f(hare))
	}

	// find mu
	*mu = 0;
	free(tortoise);
	tortoise = grid_copy(g); // tortoise = x0
	while (!grids_equal(tortoise, hare)) {
		run_input_steps(tortoise, input, input_len, input_len); // tortoise = f(tortoise)
		run_input_steps(tortoise, input, input_len, input_len); // hare = f(hare)
		++(*mu);
	}

	// find lambda
	*lambda = 1;
	// hare = f(tortoise) = f(hare)
	run_input_steps(hare, input, input_len, input_len); // hare = f(hare)
	while (!grids_equal(tortoise, hare)) {
		run_input_steps(hare, input, input_len, input_len); // hare = f(hare)
		++(*lambda);
	}
	destroy_grid(tortoise);
	destroy_grid(hare);
}


int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	len = 0;
	while (line[len] == '<' || line[len] == '>')
		++len;

	struct grid* g = create_grid(WIDTH);

	// cycle detection:
	int lambda, mu;
	floyd(g, line, len, &mu, &lambda);

	int64_t blocks_left = BLOCKS;
	// make mu steps
	run_input_steps(g, line, len, mu * len);
	blocks_left -= g->block_cnt;
	// count blocks per cycle of len lamda, and height increase per cycle
	int64_t n = g->block_cnt;
	int64_t maxy = g->maxy;
	run_input_steps(g, line, len, lambda * len);
	int64_t blocks_per_cycle = g->block_cnt - n;
	int64_t h_per_cycle = g->maxy - maxy;
	int64_t nr_cycles = blocks_left / blocks_per_cycle;
	blocks_left = blocks_left % blocks_per_cycle;
	maxy = maxy + nr_cycles * h_per_cycle;
	// last steps
	int64_t maxy_before = g->maxy;
	while (blocks_left) {
		drop_till_rest(g, line, len);
		--blocks_left;
	}
	maxy += g->maxy - maxy_before;

	printf("%"PRId64"\n", maxy + 1);

	destroy_grid(g);
	free(line);

	return 0;
}
