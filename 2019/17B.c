#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

/* max nr of [RL][x] sections */
#define MAX_PATHLEN 50
/* proglen string max 20 results in: */
#define MAX_PROGLEN 5

enum cell {
	EMPTY,
	SCAFFOLD
};

enum direction {
	RIGHT,
	UP,
	LEFT,
	DOWN
};

enum rotation {
	ROTLEFT = 1,
	ROTRIGHT = 3
};

struct subpath {
	int  program; // 0: unassigned 1:A, 2:B, 3:C
	// non-program
	enum rotation rot;
	int           fwd; // nr steps fwd
};

int dir2dx[] = {1, 0, -1, 0};
int dir2dy[] = {0, -1, 0, 1};
char* dirstr = ">^<v";

struct grid {
	int            gridw;
	int            gridh;
	int            rowcapacity; // max nr of rows reserved in mem
	enum cell*     grid;
	int            robotx;
	int            roboty;
	enum direction robotdir;
};

struct grid* create_grid() {
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = 0;
	g->gridh = 0;
	g->rowcapacity = 0;
	g->grid = NULL;
	return g;
}

void destroy_grid(struct grid* g) {
	// does not destroy entities in grid
	if (g) {
		free(g->grid);
		free(g);
	}
}

bool grid_add_line(struct grid* g, char* line) {
	// find line width
	int lw = 0;
	while (!isspace(line[lw]))
		++lw;
	if (g->gridw > 0 && g->gridw != lw) {
		//fprintf(stderr, "Line width inconsistency: line %d has width %d instead of %d\n", g->gridh, lw, g->gridw);
		return false;
	}
	if (!g->grid) {
		g->gridw = lw;
		g->gridh = 0;
		g->rowcapacity = lw; // assume square
		g->grid = malloc(g->gridw * g->rowcapacity * sizeof(enum cell));
	}
	if (g->gridh >= g->rowcapacity) {
		g->rowcapacity *= 2;
		g->grid = realloc(g->grid, g->gridw * g->rowcapacity * sizeof(enum cell));
	}
	for (int col = 0; col < g->gridw; ++col) {
		int idx = col + g->gridh * g->gridw;
		g->grid[idx] = line[col] == '.' ? EMPTY : SCAFFOLD;
		char* l = strchr(dirstr, line[col]);
		if (l) {
			g->robotx = col;
			g->roboty = g->gridh;
			g->robotdir = (enum direction)(l - dirstr);
		}
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			if (x == g->robotx && y == g->roboty)
				printf("%c", dirstr[g->robotdir]);
			else {
				int idx = y * g->gridw + x;
				printf("%c", g->grid[idx] == EMPTY ? '.' : '#');
			}
		}
		printf("\n");
	}
}


typedef int64_t word;

#define INITIAL_MEMSZ 512

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

enum mnemonics run_program(struct computer* comp, word in, bool in_valid, word* out) {
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
				if (!in_valid)
					return INP;
				// x = getchar();
				addr = computer_get_mem(comp, comp->pc + 1) + (mode[0] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, in);
				in_valid = false;
				comp->pc += 2;
				break;
			case OUT:
				*out = computer_get_parm(comp, 1, mode[0]);
				comp->pc += 2;
				return OUT;
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

void computer_copy(struct computer* dest, struct computer* from) {
	dest->mem = malloc(from->mem_sz * sizeof(word));
	memcpy(dest->mem, from->mem, from->mem_sz * sizeof(word));
	dest->mem_sz = from->mem_sz;
	dest->prog_sz = from->prog_sz;
	dest->pc = from->pc;
	dest->base = from->base;
}

bool is_scaffold(struct grid* g, int x, int y) {
	return (x >= 0 && x < g->gridw && y >= 0 && y < g->gridh && g->grid[y * g->gridw + x] == SCAFFOLD);
}

struct grid* get_grid_from_program(struct computer* comp) {
	struct computer cpy;
	computer_copy(&cpy, comp);

	struct grid* g = create_grid();

	char line[256];
	int line_idx = 0;
	word out;
	while (run_program(&cpy, 0, false, &out) == OUT) {
		char c = (char)out;
		line[line_idx++] = c;
		if (c == '\n') {
			grid_add_line(g, line);
			line_idx = 0;
		}
	}
	destroy_computer(&cpy);

	return g;
}

void obtain_path(struct grid* g, struct subpath* path, int* path_sz) {
	*path_sz = 0;
	int x = g->robotx;
	int y = g->roboty;
	int dir = g->robotdir;
	bool deadend = false;
	while (!deadend) {
		// NOTE: Algo only works if first step is rotation!!
		// rotate
		deadend = true; // until proven otherwise
		int ddir;
		for (ddir = 1; ddir <= 3; ddir += 2) {
			int d = (dir + ddir) % 4;
			if (is_scaffold(g, x + dir2dx[d], y + dir2dy[d])) {
				deadend = false;
				break;
			}
		}
		if (!deadend) {
			path[*path_sz].rot = ddir == 1 ? ROTLEFT : ROTRIGHT;
			dir = (dir + ddir) % 4;
			// count nr of steps we can take in this dir
			int steps;
			for (steps = 0; is_scaffold(g, x + dir2dx[dir], y + dir2dy[dir]); ++steps) {
				x += dir2dx[dir];
				y += dir2dy[dir];
			}
			path[*path_sz].fwd = steps;
			++(*path_sz);
		}
	}
}

void print_path(struct subpath* path, int path_sz) {
	for (int ii = 0; ii < path_sz; ++ii) {
		printf("%c%2d", path[ii].rot == ROTLEFT ? 'L' : 'R', path[ii].fwd);
		if (path[ii].program)
			printf(" %c", path[ii].program + 'A' - 1);
		printf("\n");
	}
}

int calc_spelled_len(struct subpath* path, int idx, int len) {
	// how many chars needed to spell the program?
	if (len == 0)
		return 0;
	int sum = 0;
	for (int ii = 0; ii < len; ++ii) {
		sum += 4 + (path[idx + ii].fwd >= 10 ? 1 : 0); // 4: R,x,
	}
	return sum - 1; // -1: last comma
}


void mark_program_in_path(struct subpath* path, int path_sz, int startidx, int len, int id) {
	// mark first occurrence
	for (int ii = 0; ii < len; ++ii)
		path[ii + startidx].program = id;
	for (int ii = len; ii < path_sz - len + 1; ++ii) {
		// check if starting at ii we see the entire program
		bool is_program = true;
		for (int jj = 0; is_program && jj < len; ++jj)
			is_program = path[ii + jj].program == 0 &&
				path[ii + jj].rot == path[jj + startidx].rot &&
				path[ii + jj].fwd == path[jj + startidx].fwd;
		if (is_program) // if so, mark program
			for (int jj = 0; jj < len; ++jj)
				path[ii + jj].program = id;
		// we could inc ii to skip rest of program now, but whatever
	}
}

bool find_programs(struct subpath* path, int path_sz, int* prog_sz, int start_id) { // recursive
	bool found = false;
	if (start_id == 4) { // silly base case
		// check that all path entries are assigned to a program
		for (int ii = 0; ii < path_sz; ++ii)
			if (path[ii].program == 0)
				return false;
		return true;
	}

	// find first unassigned index
	int startidx = 0;
	while (startidx < path_sz && path[startidx].program)
		++startidx;
	if (startidx < path_sz) {
		// greedy
		// find longest possible prog len
		int idx2 = startidx + 1;
		while (idx2 < path_sz && path[idx2].program == 0)
			++idx2;
		prog_sz[start_id - 1] = (idx2 - startidx > MAX_PROGLEN) ? MAX_PROGLEN : idx2 - startidx;
		while (calc_spelled_len(path, startidx, prog_sz[start_id - 1]) > 20)
			--prog_sz[start_id - 1];
		
		// mark all occurrences of the program
		found = false;
		while (!found && prog_sz[start_id - 1]) {
			mark_program_in_path(path, path_sz, startidx, prog_sz[start_id - 1], start_id);
			found = find_programs(path, path_sz, prog_sz, start_id + 1);
			if (!found) { // unmark
				for (int ii = 0; ii < path_sz; ++ii)
					if (path[ii].program == start_id)
						path[ii].program = 0;
				--prog_sz[start_id - 1]; // try smaller one
			}
		}
	}
	return found;
}

bool find_programs_top(struct subpath* path, int path_sz, int* prog_sz) {
	for (int ii = 0; ii < path_sz; ++ii)
		path[ii].program = 0; // unassigned
	return find_programs(path, path_sz, prog_sz, 1);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		return 1;
	}

	struct computer comp;
	computer_init(&comp, argv[1]);
	
	struct grid* g = get_grid_from_program(&comp);
	// show_grid(g);

	struct subpath path[MAX_PATHLEN];
	int path_sz;
	obtain_path(g, path, &path_sz);

	int prog_sz[3] = {0};
	find_programs_top(path, path_sz, prog_sz);
	// print_path(path, path_sz);

	// GENERATE COMPUTER INPUT FROM SOLUTION FOUND
	char instr[1024]; // complete input
	// main
	int instr_idx = 0;
	int path_idx = 0;
	while (path_idx < path_sz) {
		int prog_id = path[path_idx].program;
		instr_idx += sprintf(instr + instr_idx, "%c", prog_id + 'A' - 1);
		path_idx += prog_sz[prog_id - 1];
		instr_idx += sprintf(instr + instr_idx, "%c", path_idx < path_sz ? ',' : '\n');
	}
	// progs
	for (int id = 1; id <= 3; ++id) {
		for (int ii = 0; ii < path_sz; ++ii) {
			if (path[ii].program == id) {
				for (int jj = 0; jj < prog_sz[id - 1]; ++jj) {
					instr_idx += sprintf(instr + instr_idx, "%c,%d", path[ii + jj].rot == ROTLEFT ? 'L' : 'R', path[ii + jj].fwd);
					instr_idx += sprintf(instr + instr_idx, "%c", jj < prog_sz[id - 1] - 1 ? ',' : '\n');
				}
				break; // done with this program, on to the next
			}
		}
	}
	instr_idx += sprintf(instr + instr_idx, "n\n"); // video feed? : nope
	// int instr_len = instr_idx;

	// printf("%s", instr);

	word out;
	comp.mem[0] = 2;
	int in = 0;
	bool in_valid = false;
	instr_idx = 0;
	while (comp.mem[comp.pc] != HLT) {
		enum mnemonics mnem = run_program(&comp, in, in_valid, &out);
		if (mnem == OUT) {
			// printf("%c", (char)out);
			in_valid = false;
		}
		else if (mnem == INP) {
			/*
			in = getchar();
			*/
			in = instr[instr_idx++];
			in_valid = true;
		}
	}
	printf("%" PRId64 "\n", out);

	destroy_computer(&comp);
	destroy_grid(g);
	return 0;
}
