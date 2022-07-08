#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>

#include <unistd.h> /* usleep */

#define ABS(a) (((a)<0)?-(a):(a))

/************************* MINHEAP *******************************/
// NOTE: this implementation allows searching for value, and
// changing key of a value to allow A-star search !
struct minheap_el {
	int key;
	int val;
};

struct minheap {
	struct minheap_el* mem;      // array-based min heap
	unsigned int       size;     // nr elements currently in heap
	unsigned int       capacity; // capacity in nr of elements
};

struct minheap* create_minheap() {
	struct minheap* heap = malloc(sizeof(struct minheap));
	heap->capacity = 127; // 7 layers start size
	heap->mem = malloc(sizeof(struct minheap_el) * heap->capacity);
	heap->size = 0;
	return heap;
}

void destroy_minheap(struct minheap* heap) {
	if (heap)
		free(heap->mem);
	free(heap);
}

static void minheap_swap(struct minheap* heap, int idx1, int idx2) {
	struct minheap_el t = heap->mem[idx1];
	heap->mem[idx1] = heap->mem[idx2];
	heap->mem[idx2] = t;
}

static void minheap_bubble_up(struct minheap* heap, int idx) {
	int idx_parent = (idx - 1) / 2;
	while (idx != 0 && heap->mem[idx].key < heap->mem[idx_parent].key) {
		minheap_swap(heap, idx, idx_parent);
		idx = idx_parent;
		idx_parent = (idx - 1) / 2;
	}
}

static void minheap_bubble_down(struct minheap* heap, int idx) {
	int key_this = heap->mem[idx].key;
	bool done = false;
	while (!done) {
		done = true;
		int smallest_key = key_this;
		int idx_smallest = idx;
		// find smallest between idx, left and right child
		int idx_child;
		for (idx_child = (idx * 2) + 1; idx_child <= (idx * 2) + 2; ++idx_child) {
			if (idx_child < heap->size) {
				int k = heap->mem[idx_child].key;
				if (k < smallest_key) {
					smallest_key = k;
					idx_smallest = idx_child;
				}
			}
		}
		if (idx_smallest != idx) {
			done = false;
			minheap_swap(heap, idx, idx_smallest);
			idx = idx_smallest;
		}
	}
}

void minheap_insert(struct minheap* heap, int key, int val) {
	// check capacity
	if (heap->size == heap->capacity) {
		heap->capacity = 2 * heap->capacity + 1; // 1 more layer
		heap->mem = realloc(heap->mem, sizeof(struct minheap_el) * heap->capacity);
	}
	int idx = heap->size; // insertion idx
	// copy info into new last element
	heap->mem[idx].key = key;
	heap->mem[idx].val = val;
	++heap->size;
	minheap_bubble_up(heap, idx);
}

int minheap_extract(struct minheap* heap, int* val) { // returns key, copies val to *val
	if (heap->size == 0) {
		fprintf(stderr, "Error: extracting from empty heap\n");
		exit(1);
	}
	int retKey = heap->mem[0].key;
	if (val)
		*val = heap->mem[0].val;

	--heap->size;
	if (heap->size > 0) {
		// copy ex-last element to root position
		heap->mem[0] = heap->mem[heap->size];
		minheap_bubble_down(heap, 0);
	}
	return retKey;
}

bool minheap_contains_val(struct minheap* heap, int val) {
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val)
			return true;
	}
	return false;
}

bool minheap_update_key_of_val(struct minheap* heap, int key, int val) {
	// Note: only changes key of first val found that had a different key
	// returns whether change was made
	bool changed = false;
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val && heap->mem[idx].key != key) {
			int oldkey = heap->mem[idx].key;
			heap->mem[idx].key = key;
			changed = true;
			if (key < oldkey)
				minheap_bubble_up(heap, idx);
			else
				minheap_bubble_down(heap, idx);
		}
	}
	return changed;
}
/************************* END MINHEAP ***************************/


typedef int64_t word;

#define INITIAL_MEMSZ 512

#define GRIDW 64
#define GRIDH 64

enum cell {
	UNKNOWN = 0,
	EMPTY,
	WALL,
	ON_PATH /* just for showing path */
};

enum direction {
	NORTH = 0,
	SOUTH,
	WEST,
	EAST
};

int dir2dx[] = {0, 0, -1, 1};
int dir2dy[] = {-1, 1, 0, 0};
enum direction opposite[] = {SOUTH, NORTH, EAST, WEST};

struct vec2 {
	int x;
	int y;
};

struct grid {
	enum cell*  grid;
	struct vec2 droid;
	struct vec2 target;
	bool        target_known;
};

enum mnemonics {
	ADD  = 1,
	MUL  = 2,
	INP  = 3,
	OUT  = 4,
	JNE  = 5,
	JEQ  = 6,
	LT   = 7,
	EQ   = 8,
	INCB = 9,

	HLT  = 99
};

struct computer {
	word* mem;
	int   mem_sz;
	int   prog_sz; // only needed for disasm option
	int   pc;
	int   base; // "relative base"
};

bool read_int(char** pLine, word* x) {
	word num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
	*pLine = line;
	return true;
}

void computer_double_memsz(struct computer* comp) {
	comp->mem = realloc(comp->mem, 2 * comp->mem_sz * sizeof(word));
	memset(comp->mem + comp->mem_sz, 0, comp->mem_sz * sizeof(word));
	comp->mem_sz *= 2;
}

void computer_set_mem(struct computer* comp, int addr, word val) {
	while (addr >= comp->mem_sz) {
		computer_double_memsz(comp);
		printf("INFO: setting memory size to %d due to write at addres %d\n", comp->mem_sz, addr);
	}
	comp->mem[addr] = val;
}

word computer_get_mem(struct computer* comp, int addr) {
	while (addr >= comp->mem_sz) {
		computer_double_memsz(comp);
		printf("INFO: setting memory size to %d due to read from addres %d\n", comp->mem_sz, addr);
	}
	return comp->mem[addr];
}

word computer_get_parm(struct computer* comp, int pc_offset, int mode) {
	word operand = computer_get_mem(comp, comp->pc + pc_offset);
	operand += mode == 2 ? comp->base : 0;
	return mode == 1 ? operand : computer_get_mem(comp, operand);
}

void read_prog_from_file(struct computer* comp, char* fname) {
	FILE* fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "Error opening file %s\n", fname);
		return;
	}
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, fp);

	char* l = line;
	word x;
	comp->prog_sz = 0;
	while (read_int(&l, &x))
		computer_set_mem(comp, comp->prog_sz++, x);

	free(line);
	fclose(fp);
}

word run_program(struct computer* comp, int in) {
	while (computer_get_mem(comp, comp->pc) != HLT) {
		word instr = computer_get_mem(comp, comp->pc);
		int m = instr / 100;
		int opcode = instr % 100;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		int addr;
		word out;
		switch (opcode) {
			case ADD:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) + computer_get_parm(comp, 2, mode[1]));
				comp->pc += 4;
				break;
			case MUL:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) * computer_get_parm(comp, 2, mode[1]));
				comp->pc += 4;
				break;
			case INP:
				addr = computer_get_mem(comp, comp->pc + 1) + (mode[0] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, in);
				comp->pc += 2;
				break;
			case OUT:
				// printf("(OUT @ pc=%d) %" PRId64 "\n", comp->pc, computer_get_parm(comp, 1, mode[0]));
				out = computer_get_parm(comp, 1, mode[0]);
				comp->pc += 2;
				return out;
			case JNE:
				if (computer_get_parm(comp, 1, mode[0]) != 0)
					comp->pc = computer_get_parm(comp, 2, mode[1]);
				else
					comp->pc += 3;
				break;
			case JEQ:
				if (computer_get_parm(comp, 1, mode[0]) == 0)
					comp->pc = computer_get_parm(comp, 2, mode[1]);
				else
					comp->pc += 3;
				break;
			case LT:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) < computer_get_parm(comp, 2, mode[1]) ? 1 : 0);
				comp->pc += 4;
				break;
			case EQ:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) == computer_get_parm(comp, 2, mode[1]) ? 1 : 0);
				comp->pc += 4;
				break;
			case INCB:
				comp->base += computer_get_parm(comp, 1, mode[0]);
				comp->pc += 2;
				break;
			default:
				fprintf(stderr, "Invalid opcode %d @ %d\n", opcode, comp->pc);
				exit(1);
		}
	}
	return HLT;
}

void computer_init(struct computer* comp, char* progfname) {
	comp->mem_sz = INITIAL_MEMSZ;
	comp->mem = calloc(comp->mem_sz, sizeof(word));
	read_prog_from_file(comp, progfname);
	comp->pc = 0;
	comp->base = 0;
}

void destroy_computer(struct computer* comp) {
	free(comp->mem);
}

void show_grid(struct grid* g, int sleeptime_us) {
	for (int y = 0; y < GRIDH; ++y) {
		for (int x = 0; x < GRIDW; ++x) {
			int idx = y * GRIDW + x;
			bool isdroid = (x == g->droid.x && y == g->droid.y);
			bool istarget = g->target_known && (x == g->target.x && y == g->target.y);

			if (isdroid && istarget)
				printf("@");
			else if (istarget)
				printf("$");
			else if (isdroid)
				printf("D");
			else {
				char c;
				switch (g->grid[idx]) {
					case EMPTY:
						c = '.';
						break;
					case WALL:
						c = '#';
						break;
					case ON_PATH:
						c = 'o';
						break;
					default:
						c = ' ';
				}
				printf("%c", c);
			}
		}
		printf("\n");
	}
	usleep(sleeptime_us);
}

bool try_move(enum direction d, struct grid* g, struct computer* comp) {
	int nx = g->droid.x + dir2dx[d];
	int ny = g->droid.y + dir2dy[d];
	if (nx < 0 || nx >= GRIDW || ny < 0 || ny >= GRIDH) {
		printf("ERROR: running off the grid @ %d, %d\n", nx, ny);
		return false;
	}
	int nidx = ny * GRIDW + nx;

	word out = run_program(comp, d + 1); // +1: 1-based dirs used in intcode

	if (out == 0) {
		g->grid[nidx] = WALL;
		return false;
	}
	g->droid.x = nx;
	g->droid.y = ny;
	g->grid[nidx] = EMPTY;
	if (out == 2) {
		g->target.x = nx;
		g->target.y = ny;
		g->target_known = true;
	}
	return true;
}

/************************* A-STAR SEARCH *************************/
int hscore(int dx, int dy) {
	dx = dx < 0 ? -dx : dx;
	dy = dy < 0 ? -dy : dy;
	return dx + dy;
}

struct path_idx {
	int idx;
	struct path_idx* next;
};

struct path_idx* find_path(struct grid* g, int fx, int fy, int tx, int ty) {
	// A-star search algo
	// a nodes are (x,y), encoded in integer:
	// (y * w + x)  (0<=y<h, 0<=x<w)
	int w = GRIDW;
	int h = GRIDH;
	int idx = fy * w + fx; // current state
	int target_idx = ty * GRIDW + tx;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	int* camefrom = malloc(w * h * sizeof(int));
	//int* fscore = malloc(w * h * sizeof(int)); // No need since fscore is key of open_set
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
		//fscore[ii] = INT_MAX;
	}

	gscore[idx] = 0;
	//fscore[idx] = hscore(map, 0, 0, TORCH);
	fscore = hscore(fx - tx, fy - ty);

	//minheap_insert(open_set, fscore[idx], idx);
	minheap_insert(open_set, fscore, idx);

	struct path_idx* path = NULL;
	int startidx = idx;
	while (path == NULL && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		if (idx == target_idx) { // found it: recreate path
			while (idx != startidx) {
				struct path_idx* p = malloc(sizeof(struct path_idx));
				p->idx = idx;
				p->next = path;
				path = p;
				idx = camefrom[idx];
			}
		}
		else {
			// for each neighbor of current
			for (int dir = 0; dir < 4; ++dir) {
				int nx = x + dir2dx[dir];
				int ny = y + dir2dy[dir];
				if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
					int nidx = ny * w + nx;
					// check if we can travel there
					if (g->grid[nidx] == EMPTY) {
						int tentative_gscore = gscore[idx] + 1;
						if (tentative_gscore < gscore[nidx]) {
							camefrom[nidx] = idx;
							gscore[nidx] = tentative_gscore;
							fscore = tentative_gscore + hscore(nx - tx, ny - tx);
							if (!minheap_contains_val(open_set, nidx))
								minheap_insert(open_set, fscore, nidx);
							else
								minheap_update_key_of_val(open_set, fscore, nidx); // Nasty...
						}
					}
				}
			}
		}
	}

	destroy_minheap(open_set);
	free(gscore);
	free(camefrom);
	return path;
}

bool has_unknown_neighbors(struct grid* g) {
	for (int dir = 0; dir < 4; ++dir) {
		int x = g->droid.x + dir2dx[dir];
		int y = g->droid.y + dir2dy[dir];
		int idx = y * GRIDW + x;
		if (x >= 0 && x < GRIDW && y >= 0 && y < GRIDH)
			if (g->grid[idx] == UNKNOWN)
				return true;
	}
	return false;
}

void explore_grid(struct grid* g, struct computer* comp, bool show) {
	// makes droid walk through grid until all reachable cells visited
	// TODO: no need to prioritize on dist to start point? Use regular stack e.g.
	struct minheap* mindistheap = create_minheap(); // key: dist to initial pos, val: idx
	int startx = g->droid.x;
	int starty = g->droid.y;
	int idx = starty * GRIDW + startx;
	minheap_insert(mindistheap, 0, idx);

	int twait = 10000;

	if (show)
		show_grid(g, twait);

	while (mindistheap->size) {
		minheap_extract(mindistheap, &idx);
		int ty = idx / GRIDW;
		int tx = idx % GRIDW;
		// printf("Finding path from %d,%d to %d,%d\n", g->droid.x, g->droid.y, tx, ty);
		struct path_idx* path = find_path(g, g->droid.x, g->droid.y, tx, ty);
		while (path) { // walk path
			int nidx = path->idx;
			int dx = (nidx % GRIDW) - g->droid.x;
			int dy = (nidx / GRIDW) - g->droid.y;
			enum direction d = dx == 0 ? (dy == 1 ? SOUTH : NORTH) : (dx == 1 ? EAST : WEST);
			if (!try_move(d, g, comp)) {
				fprintf(stderr, "Could not move along shortest path!!\n");
				exit(1);
			}
			if (show)
				show_grid(g, twait / 4);

			struct path_idx* p = path;
			path = path->next;
			free(p);
		}
		if (g->droid.x != tx || g->droid.y != ty) {
			fprintf(stderr, "Ended up at %d,%d instead of %d,%d\n", g->droid.x, g->droid.y, tx, ty);
			exit(1);
		}
		// We are now at a cell that has UNKNOWN neighbors
		// check 4 neighbors
		for (int dir = 0; dir < 4; ++dir) {
			int nx = g->droid.x + dir2dx[dir];
			int ny = g->droid.y + dir2dy[dir];
			int nidx = ny * GRIDW + nx;
			if (g->grid[nidx] == UNKNOWN) {
				// xplore the unknown
				if (try_move(dir, g, comp)) {
					if (show)
						show_grid(g, twait);
					if (has_unknown_neighbors(g)) {
						int dist = ABS(nx - startx) + ABS(ny - starty);
						minheap_insert(mindistheap, dist, g->droid.y * GRIDW + g->droid.x);
					}
					try_move(opposite[dir], g, comp); // move back
				}
				if (show)
					show_grid(g, twait);
			}
		}
	}

	destroy_minheap(mindistheap);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		return 1;
	}
	struct computer comp;
	computer_init(&comp, argv[1]);
	
	struct grid grid;
	grid.grid = calloc(GRIDW * GRIDH, sizeof(enum cell));
	int startx = GRIDW / 2;
	int starty = GRIDH / 2;
	grid.droid.x = startx;
	grid.droid.y = starty;
	grid.grid[grid.droid.y * GRIDW + grid.droid.x] = EMPTY;
	grid.target_known = false;

	explore_grid(&grid, &comp, false);
	int pathlen = 0;
	if (!grid.target_known)
		fprintf(stderr, "Target was not encountered\n");
	else {
		// find path from startpos to target
		struct path_idx* path = find_path(&grid, startx, starty, grid.target.x, grid.target.y);
		// mark path
		while (path) { // walk path
			++pathlen;
			grid.grid[path->idx] = ON_PATH;
			struct path_idx* p = path;
			path = path->next;
			free(p);
		}
	}
	//show_grid(&grid, 0);
	printf("%d\n", pathlen);

	destroy_computer(&comp);
	return 0;
}
