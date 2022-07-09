#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

enum cell {
	EMPTY,
	SCAFFOLD
};

struct grid {
	int          gridw;
	int          gridh;
	int          rowcapacity; // max nr of rows reserved in mem
	enum cell*   grid;
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
	}
	++g->gridh;
	return true;
}

void show_grid(struct grid* g) {
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			printf("%c", g->grid[idx] == EMPTY ? '.' : '#');
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

enum mnemonics run_program(struct computer* comp, word* out) {
	while (computer_get_mem(comp, comp->pc) != HLT) {
		word instr = computer_get_mem(comp, comp->pc);
		int m = instr / 100;
		int opcode = instr % 100;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		int x;
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
				printf("Value: ");
				scanf("%d", &x);
				addr = computer_get_mem(comp, comp->pc + 1) + (mode[0] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, x);
				comp->pc += 2;
				break;
			case OUT:
				// printf("(OUT @ pc=%d) %" PRId64 "\n", comp->pc, computer_get_parm(comp, 1, mode[0]));
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

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		return 1;
	}

	struct computer comp;
	computer_init(&comp, argv[1]);
	
	struct grid* grid = create_grid();

	char line[256];
	int line_idx = 0;
	word out;
	while (run_program(&comp, &out) == OUT) {
		char c = (char)out;
		line[line_idx++] = c;
		if (c == '\n') {
			grid_add_line(grid, line);
			line_idx = 0;
		}
	}
	// show_grid(grid);

	// count intersections
	int sum = 0;
	for (int y = 1; y < grid->gridh - 1; ++y) {
		for (int x = 1; x < grid->gridw - 1; ++x) {
			int idx = y * grid->gridw + x;
			if (grid->grid[idx] == SCAFFOLD &&
					grid->grid[idx - grid->gridw] == SCAFFOLD && grid->grid[idx + grid->gridw] == SCAFFOLD &&
					grid->grid[idx - 1] == SCAFFOLD && grid->grid[idx + 1] == SCAFFOLD) {
				sum += x * y;
			}
		}
	}
	printf("%d\n", sum);

	destroy_computer(&comp);
	destroy_grid(grid);
	return 0;
}
