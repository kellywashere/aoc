#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct intlist_el {
	int x;
	struct intlist_el* next;
};

struct intlist {
	struct intlist_el* head;
	struct intlist_el* tail;
};

struct intlist* create_intlist(void) {
	struct intlist* l = malloc(sizeof(struct intlist));
	l->head = NULL;
	l->tail = NULL;
	return l;
}

void destroy_intlist(struct intlist* l) {
	if (l) {
		struct intlist_el* e = l->head;
		while (e) {
			l->head = e->next;
			free(e);
			e = l->head;
		}
		free(l);
	}
}

bool intlist_is_empty(struct intlist* l) {
	return l->head == NULL;
}

/*
void add_to_front(struct intlist* l, struct intlist_el* el) {
	el->next = l->head;
	l->head = el;
	if (!l->tail)
		l->tail = el;
}
*/

void intlist_add_to_back(struct intlist* l, struct intlist_el* el) {
	el->next = NULL;
	if (l->tail) {
		l->tail->next = el;
		l->tail = el;
	}
	else {
		l->head = el;
		l->tail = el;
	}
}

struct intlist_el* intlist_remove_from_front(struct intlist* l) {
	struct intlist_el* el = l->head;
	if (el) {
		l->head = el->next;
		el->next = NULL;
		if (!l->head) // list became empty
			l->tail = NULL;
	}
	else
		fprintf(stderr, "Trying to remove from empty list!\n");
	return el;
}

void print_intlist(struct intlist* l) {
	for (struct intlist_el* e = l->head; e; e = e->next)
		printf("%d ", e->x);
	printf("\n");
}

int read_int(char** pLine) {
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
	return num;
}

void play_round(struct intlist* cards[]) {
	struct intlist_el* c[2];
	c[0] = intlist_remove_from_front(cards[0]);
	c[1] = intlist_remove_from_front(cards[1]);
	if (c[0]->x > c[1]->x) {
		intlist_add_to_back(cards[0], c[0]);
		intlist_add_to_back(cards[0], c[1]);
	}
	else {
		intlist_add_to_back(cards[1], c[1]);
		intlist_add_to_back(cards[1], c[0]);
	}
}

int calc_score(struct intlist* l) {
	int nr_cards = 0;
	struct intlist_el* el = l->head;
	while (el) {
		++nr_cards;
		el = el->next;
	}
	int score = 0;
	el = l->head;
	while (el) {
		score += nr_cards * el->x;
		el = el->next;
		--nr_cards;
	}
	return score;
}

int main(int argc, char* argv[]) {
	struct intlist* cards[2];
	cards[0] = create_intlist();
	cards[1] = create_intlist();

	char *line = NULL;
	size_t len = 0;
	int player = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		char* l = line;
		if (*l == 'P') {
			player = read_int(&l) - 1;
			if (player >= 2) {
				fprintf(stderr, "Invalid player #%d\n", player);
				return 1;
			}
		}
		else {
			struct intlist_el* el = malloc(sizeof(struct intlist_el));
			el->x = read_int(&l);
			el->next = NULL;
			intlist_add_to_back(cards[player], el);
		}
	}
	free(line);

	while (!intlist_is_empty(cards[0]) && !intlist_is_empty(cards[1])) {
		play_round(cards);
		/*
		printf("Cards:\n");
		print_intlist(cards[0]);
		print_intlist(cards[1]);
		*/
	}

	int winner = intlist_is_empty(cards[0]) ? 1 : 0;
	printf("%d\n", calc_score(cards[winner]));


	destroy_intlist(cards[0]);
	destroy_intlist(cards[1]);
	return 0;
}
