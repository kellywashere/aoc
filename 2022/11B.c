#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define MAX_MONKEYS 16
#define MAX_ITEMS   64

#define TURNS 10000

enum operator {
	MUL,
	ADD
};

struct monkey {
	enum operator operator;
	int           op_param; // 0 means: `old`
	int           test_mod;
	int           true_throw;
	int           false_throw;
	uint64_t      items[MAX_ITEMS];
	int           nr_items;

	int           items_inspected;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	*x = num;
	return true;
}

bool read_monkey(struct monkey* m, FILE* f) {
	char *line = NULL;
	size_t len = 0;
	// FSM flags
	bool monkey_read = false;
	bool reading_monkey = false;
	while (!monkey_read && getline(&line, &len, stdin) != -1) {
		char* l = line;
		if (empty_line(line))
			continue;
		while (isspace(*l))
			++l;
		if (!reading_monkey && strncmp(l, "Monkey", 6) != 0)
			return false; // first line does not start with `Monkey` --> go back
		reading_monkey = true;
		if (!strncmp(l, "Starting", 8)) {
			m->nr_items = 0;
			int x;
			while (read_int(&l, &x))
				m->items[m->nr_items++] = x;
		}
		else if (!strncmp(l, "Operation", 9)) {
			while (*l != '*' && *l != '+')
				++l;
			m->operator = *l == '*' ? MUL : ADD;
			read_int(&l, &m->op_param);
		}
		else if (!strncmp(l, "Test", 4)) {
			read_int(&l, &m->test_mod);
		}
		else if (!strncmp(l, "If true", 7)) {
			read_int(&l, &m->true_throw);
		}
		else if (!strncmp(l, "If false", 8)) {
			read_int(&l, &m->false_throw);
			monkey_read = true; // done
		}
	}
	m->items_inspected = 0;
	free(line);
	return monkey_read;
}

void print_monkey_data(struct monkey* m) {
	printf("  Starting items: ");
	for (int ii = 0; ii < m->nr_items - 1; ++ii)
		printf("%"PRIu64", ", m->items[ii]);
	if (m->nr_items)
		printf("%"PRIu64"\n", m->items[m->nr_items - 1]);
	printf("  Operation: new = old %c ", m->operator == MUL ? '*' : '+');
	if (m->op_param)
		printf("%d\n", m->op_param);
	else
		printf("old\n");
	printf("  Test: divisible by %d\n", m->test_mod);
	printf("    If true: throw to monkey %d\n", m->true_throw);
	printf("    If false: throw to monkey %d\n", m->false_throw);
}

void monkey_turn(struct monkey* monkeys, int idx, uint64_t mod) {
	struct monkey* m = &monkeys[idx];
	for (int ii = 0; ii < m->nr_items; ++ii) {
		uint64_t x = m->items[ii];
		// do operation
		uint64_t param = m->op_param;
		if (param == 0)
			param = x; //`old`
		x = m->operator == MUL ? x * param : x + param;
		x = x % mod;
		bool test = x % m->test_mod == 0;
		int new_idx = test ? m->true_throw : m->false_throw;
		struct monkey* to = &monkeys[new_idx];
		to->items[to->nr_items++] = x;
	}
	m->items_inspected += m->nr_items;
	m->nr_items = 0;
}

int main(int argc, char* argv[]) {
	struct monkey monkeys[MAX_MONKEYS];
	int nr_monkeys = 0;
	while (read_monkey(&monkeys[nr_monkeys], stdin))
		++nr_monkeys;

	uint64_t mod = 1;
	for (int ii = 0; ii < nr_monkeys; ++ii)
		mod *= monkeys[ii].test_mod;

	for (int r = 0; r < TURNS; ++r) {
		for (int ii = 0; ii < nr_monkeys; ++ii)
			monkey_turn(monkeys, ii, mod);
	}

	// find largest 2
	uint64_t l[2] = {0}; // l[0] >= l[1]
	for (int ii = 0; ii < nr_monkeys; ++ii) {
		int x = monkeys[ii].items_inspected;
		if (x >= l[0]) {
			l[1] = l[0];
			l[0] = x;
		}
		else if (x >= l[1])
			l[1] = x;
	}

	// printf("%d * %d = %d\n", l[0], l[1], l[0]*l[1]);
	printf("%"PRIu64"\n", l[0] * l[1]);

	return 0;
}
