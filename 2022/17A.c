#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define WIDTH 7
#define HEIGHT 10000
#define BLOCKS 2022

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
	int   h;
	int*  grid; // each row represented as bitfield

	int   maxy;
	int   block_idx;
	int   blockx;
	int   blocky;
};

void show_grid(struct grid* g) {
	printf("block %d @ %d,%d\n", g->block_idx, g->blockx, g->blocky);
	for (int y = g->maxy + 7; y >= 0; --y) {
		printf("|");
		for (int x = 0; x < g->w; ++x) {
			char c = '.';
			int by = y - g->blocky;
			int bx = x - g->blockx;
			if (0 <= by && by < 4 && 0 <= bx && bx < 4) {
				int b = blocks[g->block_idx].row[by];
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

void spawn_block(struct grid* g) {
	g->block_idx = (g->block_idx + 1) % (sizeof(blocks) / sizeof(blocks[0]));
	g->blockx = 2;
	g->blocky = g->maxy + 4;
}

void freeze_block(struct grid* g) {
	for (int dy = 0; dy < 4; ++dy) {
		int b = blocks[g->block_idx].row[dy];
		int shl = g->w - 4 - g->blockx;
		b = shl >= 0 ? (b << shl) : (b >> (-shl));
		g->grid[g->blocky + dy] |= b;
		if (b && g->blocky + dy > g->maxy)
			g->maxy = g->blocky + dy;
	}
}

struct grid* create_grid(int w, int h) {
	struct grid* g = malloc(sizeof(struct grid));
	g->w = w;
	g->h = h;
	g->grid = calloc(w * h, sizeof(int));
	g->maxy = -1;
	g->block_idx = -1;
	spawn_block(g);
	return g;
}

void destroy_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool check_overlap(struct grid* g) {
	if ((g->blocky < 0) ||
			(g->blockx < 0) ||
			(g->blockx + blocks[g->block_idx].w > g->w))
		return true;
	for (int dy = 0; dy < 4; ++dy) {
		int b = blocks[g->block_idx].row[dy];
		int shl = g->w - 4 - g->blockx;
		b = shl >= 0 ? (b << shl) : (b >> (-shl));
		if ( (b & g->grid[g->blocky + dy]) )
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

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	len = 0;
	while (line[len] == '<' || line[len] == '>')
		++len;

	struct grid* g = create_grid(WIDTH, HEIGHT);
	int line_idx = 0;

	for (int ii = 0; ii < BLOCKS; ++ii) {
		bool rest = false;
		do {
			int dx = line[line_idx] == '<' ? -1 : 1;
			line_idx = (line_idx + 1) % len;
			push_block(g, dx);
			rest = !drop_block(g);
		} while (!rest);
		freeze_block(g);
		spawn_block(g);
		//show_grid(g);
	}
	printf("%d\n", g->maxy + 1);
	destroy_grid(g);
	free(line);

	return 0;
}
