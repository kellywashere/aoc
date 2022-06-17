#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

struct state {
	int   offset; // .state[0] has original idx `offset`
	bool* state;
	int   len;    // length of state
};

void gen_nextstate(struct state* cur, struct state* next, bool* rules) {
	// find first plant in state
	int first = 0;
	while (first < cur->len && !cur->state[first])
		++first;
	if (first == cur->len) { // empty state not handled yet
		printf("TODO\n");
		return;
	}
	next->offset = cur->offset + first - 2; // first idx that can posssibly contain plant
	int last = cur->len - 1;
	while (last >= first && !cur->state[last])
		--last;
	int len = last - first + 5;
	if (next->state == NULL || len > next->len) {
		next->len = len + 16; // just add some margin for next steps
		if (next->state == NULL)
			next->state = malloc(next->len * sizeof(bool));
		else
			next->state = realloc(next->state, next->len * sizeof(bool));
	}
	for (int ii = 0; ii < next->len; ++ii) {
		int rule = 0;
		// idx_next = ii + next_offset
		// idx_cur = idx_next - 2 ... idx_next + 2
		// jj = idx_cur - cur_offset = ii + next_offset - cur_offset - 2 ... ii + next_offset - cur_offset + 2
		// jj = ii + first - 4 ... ii + first
		for (int jj = ii + first - 4; jj <= ii + first; ++jj)
			rule = (rule << 1) + ((jj >= 0 && jj < cur->len && cur->state[jj]) ? 1 : 0);
		next->state[ii] = rules[rule];
	}
}

void print_state(struct state* state) {
	printf("%6d ", state->offset);
	for (int ii = 0; ii < state->len; ++ii)
		printf("%c", state->state[ii] ? '#' : '.');
	printf("\n");
}

int str2ruleidx(char* str) {
	int rule = 0;
	for (int ii = 0; ii < 5; ++ii)
		rule = (rule << 1) + (str[ii] == '#' ? 1 : 0);
	return rule;
}

int first_idx(struct state* s) {
	int f = 0;
	while (f < s->len && !s->state[f])
		++f;
	return f;
}
bool states_are_same_except_offset(struct state* s1, struct state* s2) {
	int idx1 = first_idx(s1);
	int idx2 = first_idx(s2);
	for (; idx1 < s1->len || idx2 < s2->len; ++idx1, ++idx2) {
		bool v1 = idx1 < s1->len ? s1->state[idx1] : false;
		bool v2 = idx2 < s2->len ? s2->state[idx2] : false;
		if (v1 != v2)
			return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	int maxsteps = 1000; // until same state detected
	int64_t totsteps = 50000000000; // puzzle nr

	char *line = NULL;
	size_t len = 0;

	// read initial state
	// 1. find initial length
	getline(&line, &len, stdin);
	char* l = line;
	while (*l && *l != '.' && *l != '#')
		++l;
	char* l2 = l;
	while (*l2 && (*l2 == '.' || *l2 == '#'))
		++l2;
	int lenzero  = (int)(l2 - l);
	// 2. reserve mem
	struct state cur;
	cur.offset = 0;
	cur.state = calloc(lenzero, sizeof(bool));
	cur.len = lenzero;
	// 3. copy initial state in center of state vector
	for (int ii = 0; ii < lenzero; ++ii)
		cur.state[ii] = l[ii] == '#';

	// read rules
	bool rules[32] = {false};
	getline(&line, &len, stdin); // empty line
	while (getline(&line, &len, stdin) != -1)
		rules[str2ruleidx(line)] = line[9] == '#';
	free(line);

	// simulate
	struct state next;
	next.state = NULL;
	struct state* pCur = &cur;
	struct state* pNext = &next;
	//print_state(pCur);
	int step = 0;
	do {
		++step;
		gen_nextstate(pCur, pNext, rules);
		struct state* tmp = pCur;
		pCur = pNext;
		pNext = tmp;
		//printf("%5d", step);
		//print_state(pCur);
	} while (step < maxsteps && !states_are_same_except_offset(pCur, pNext));
	struct state* pPrev = pNext; // due to swap, pNext is actually prev state. rename for clarity
	int dIdx = (first_idx(pCur) + pCur->offset) - (first_idx(pPrev) + pPrev->offset);
	// count plants, and calc sum of idx'es
	int nrplants = 0;
	int64_t sum = 0;
	for (int ii = 0; ii < pCur->len; ++ii) {
		nrplants += pCur->state[ii] ? 1 : 0;
		sum += pCur->state[ii] ? ii : 0;
	}
	int64_t offset_final = (totsteps - step) * dIdx + pCur->offset;
	sum += offset_final * nrplants;
	printf("%" PRIi64 "\n", sum);

	return 0;
}
