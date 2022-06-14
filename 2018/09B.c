#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

struct marble {
	int            nr;
	struct marble* next;
	struct marble* prev;
};

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

void insert_marble_after(struct marble* after_marble, struct marble* m) {
	// adds marble `m` after `after_marble`
	m->next = after_marble->next;
	m->prev = after_marble;
	after_marble->next = m;
	m->next->prev = m;
}

struct marble* remove_marble(struct marble* cur_marble) {
	// remove cur_marble
	// returns marble that came after cur_marble, or NULL when last marble removed
	struct marble* p = cur_marble->prev;
	struct marble* n = cur_marble->next;
	if (cur_marble->next == cur_marble) { // last marble
		n = NULL;
	}
	else {
		p->next = n;
		n->prev = p;
	}
	cur_marble->next = NULL;
	cur_marble->prev = NULL;
	return n;
}

void print_marbles(struct marble* m) {
	if (m) {
		struct marble* n = m;
		do {
			printf("%d ", n->nr);
			n = n->next;
		} while (n != m);
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	int players, last_marble;
	if (argc > 2) {
		players = atoi(argv[1]);
		last_marble = atoi(argv[2]);
	}
	else {
		// read puzzle data from stdin
		char *line = NULL;
		size_t len = 0;
		getline(&line, &len, stdin);
		char* l = line;
		players = read_int(&l);
		last_marble = read_int(&l);
		free(line);
	}
	last_marble *= 100;

	uint64_t* player_scores = calloc(players, sizeof(uint64_t));

	// put down 0 marble
	struct marble* cur_marble = malloc(sizeof(struct marble));
	// struct marble* zero_marble = cur_marble; //  TODO: remove this
	cur_marble->nr   = 0;
	cur_marble->next = cur_marble;
	cur_marble->prev = cur_marble;

	int player = 0;
	for (int m_nr = 1; m_nr <= last_marble; ++m_nr) {
		if (m_nr % 23 == 0) {
			for (int ii = 0; ii < 7; ++ii)
				cur_marble = cur_marble->prev;
			struct marble* m = cur_marble;
			player_scores[player] += m_nr + m->nr;
			cur_marble = remove_marble(cur_marble);
			free(m);
		}
		else {
			struct marble* m = malloc(sizeof(struct marble));
			m->nr = m_nr;
			cur_marble = cur_marble->next;
			insert_marble_after(cur_marble, m);
			cur_marble = cur_marble->next;
		}
		// print_marbles(zero_marble);
		player = (player + 1) % players;
	}

	// clean up
	while (cur_marble) {
		struct marble* m = cur_marble;
		cur_marble = remove_marble(m);
		free(m);
	}

	uint64_t hiscore = 0;
	for (int player = 0; player < players; ++player)
		hiscore = player_scores[player] > hiscore ? player_scores[player] : hiscore;
	printf("%" PRIu64 "\n", hiscore);
	free(player_scores);
	return 0;
}
