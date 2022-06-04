#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

#define MAXPROGLEN    1024
#define MSG_QUEUE_LEN 1023

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

int read_reg(char** pLine) {
	char* line = *pLine;
	while (*line && (*line < 'a' || *line > 'z'))
		++line;
	int regnr = *line - 'a';
	*pLine = line + 1;
	return regnr;
}

int read_reg_or_int(char** pLine, bool* is_int) {
	char* l = *pLine;
	int x;
	while (*l && !(*l == '-' || (*l >= '0' && *l <= '9') || (*l >= 'a' && *l <= 'z')))
		++l;
	if (*l >= 'a' && *l <= 'z') {
		*is_int = false;
		x = read_reg(&l);
	}
	else {
		*is_int = true;
		x = read_int(&l);
	}
	*pLine = l;
	return x;
}

enum opcode {
	SND,
	SET,
	ADD,
	MUL,
	MOD,
	RCV,
	JGZ
};

struct instruction {
	enum opcode opcode;
	bool        immediate1;
	int         op1;
	bool        immediate2;
	int         op2;
};

struct program {
	int                proglen; // nr of instructions
	struct instruction instr[MAXPROGLEN];
};

struct computer {
	int64_t reg[26];
	int     pc;
	struct  program* program;
	// msg queue
	int64_t msg[MSG_QUEUE_LEN + 1]; // +1: front == back means empty
	int64_t msg_front;
	int64_t msg_back;
	// answer problem
	int count_snd;
};

void read_program_from_stdin(struct program* program) {
	int idx = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		while (*l && isspace(*l))
			++l;
		if (!strncmp(l, "snd", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = SND;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			++idx;
		}
		else if (!strncmp(l, "set", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = SET;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "add", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = ADD;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "mul", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = MUL;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "mod", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = MOD;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "rcv", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = RCV;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			++idx;
		}
		else if (!strncmp(l, "jgz", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JGZ;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
	}
	program->proglen = idx;
	free(line);
}

bool is_msg_queue_empty(struct computer* computer) {
	return computer->msg_front == computer->msg_back;
}

bool is_msg_queue_full(struct computer* computer) {
	return (computer->msg_back - computer->msg_front + MSG_QUEUE_LEN + 1) % (MSG_QUEUE_LEN + 1) == MSG_QUEUE_LEN;
}

void snd_msg(struct computer* computer, int64_t msg) {
	if (!is_msg_queue_full(computer)) {
		computer->msg[computer->msg_back++] = msg;
		computer->msg_back %= MSG_QUEUE_LEN + 1;
	}
}

int64_t rcv_msg(struct computer* computer) {
	int64_t m = 0;
	if (!is_msg_queue_empty(computer)) {
		m = computer->msg[computer->msg_front++];
		computer->msg_front %= MSG_QUEUE_LEN + 1;
	}
	return m;
}

bool pc_out_of_range(struct computer* computer) {
	return (computer->pc < 0 || computer->pc >= computer->program->proglen);
}

bool is_blocked(struct computer* computer, struct computer* other) {
	struct instruction* instr = computer->program->instr + computer->pc;
	return ( pc_out_of_range(computer) ||
	         (instr->opcode == RCV && is_msg_queue_empty(computer)) ||
	         (instr->opcode == SND && is_msg_queue_full(other)) );
}

/* Run computer as long until
 * 1. snd would cause other computer to have msg queue overflow
 * 2. rcv with empty msg queue
 * 3. jmp outside of program
 */
void run_computer(struct computer* computer, struct computer* other) {
	int64_t offs;
	int64_t x;
	while (!is_blocked(computer, other)) {
		struct instruction* instr = computer->program->instr + computer->pc++;
		switch (instr->opcode) {
			case SND:
				snd_msg(other, instr->immediate1 ? instr->op1 : computer->reg[instr->op1]);
				++computer->count_snd;
				break;
			case SET:
				computer->reg[instr->op1] = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				break;
			case ADD:
				x = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				computer->reg[instr->op1] += x;
				break;
			case MUL:
				x = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				computer->reg[instr->op1] *= x;
				break;
			case MOD:
				x = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				computer->reg[instr->op1] %= x;
				break;
			case RCV:
				computer->reg[instr->op1] = rcv_msg(computer);
				break;
			case JGZ:
				x = instr->immediate1 ? instr->op1 : computer->reg[instr->op1];
				offs = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				if (x > 0)
					computer->pc += offs - 1; // -1: already inc-ed by 1 before
				break;
		}
	}
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer[2];

	read_program_from_stdin(&program);

	for (int comp = 0; comp < 2; ++comp) {
		for (int ii = 0; ii < 26; ++ii)
			computer[comp].reg[ii] = 0;
		computer[comp].reg['p' - 'a'] = comp;
		computer[comp].pc = 0;
		computer[comp].program = &program;
		computer[comp].count_snd = 0;
	}

	do {
		run_computer(&computer[0], &computer[1]);
		run_computer(&computer[1], &computer[0]);
	} while (!is_blocked(&computer[0], &computer[1]) || !is_blocked(&computer[1], &computer[0]));

	printf("%d\n", computer[1].count_snd);

	return 0;
}
