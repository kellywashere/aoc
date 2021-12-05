#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
	int         val;
	struct node* next;
};

// debug:
void print_list(struct node* n) {
	while (n) {
		printf("%d, ", n->val);
		n = n->next;
	}
	printf("\n");
}

int bit_count(struct node* n, int pos, int nrbits) {
	// >0 when 1 is most common, <0 for 0 most common, =0 for equal
	int shift = nrbits - 1 - pos; // to extract relevant bit
	int count = 0;
	while (n != NULL) {
		int b = (n->val >> shift) & 1;
		count += 2 * b - 1; // +1 for 1, -1 for 0
		n = n->next;
	}
	return count;
}

struct node* filter_list(struct node* head, int pos, int nrbits, int targetb) {
	// only keep ndes with bit at pos == targetb
	// returns (new?) head
	int shift = nrbits - 1 - pos; // to extract relevant bit
	struct node* n = head;
	struct node* nprev = NULL;
	struct node* nnext;
	while (n != NULL) {
		int b = (n->val >> shift) & 1;
		nnext = n->next;
		if (b != targetb) {
			if (n == head)
				head = nnext;
			else
				nprev->next = nnext;
			free(n);
		}
		else
			nprev = n;
		n = nnext;
	}
	return head;
}

struct node* filter_oxgen_list(struct node* head, int pos, int nrbits) {
	int count = bit_count(head, pos, nrbits);
	int filterb = count >= 0 ? 1 : 0;
	return filter_list(head, pos, nrbits, filterb);
}

struct node* filter_co2scrub_list(struct node* head, int pos, int nrbits) {
	int count = bit_count(head, pos, nrbits);
	int filterb = count >= 0 ? 0 : 1;
	return filter_list(head, pos, nrbits, filterb);
}

int count_bits(const char* str) {
	const char* s = str;
	while (*s == '0' || *s == '1')
		++s;
	return (s - str);
}

int main(int argc, char* argv[]) {
	int nrbits = 0;
	int pos;
	int oxgen = 0;
	int co2scrub = 0;
	struct node* oxgen_list = NULL;
	struct node* co2scrub_list = NULL;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (!nrbits)
			nrbits = count_bits(line);
		int x = strtol(line, NULL, 2);
		// add to oxgen list
		struct node* n = malloc(sizeof(struct node));
		n->val = x;
		n->next = oxgen_list;
		oxgen_list = n;
		// add to co2scrub list
		n = malloc(sizeof(struct node));
		n->val = x;
		n->next = co2scrub_list;
		co2scrub_list = n;
	}
	free(line);

	// filter oxgen list
	// printf("Before:\n");
	// print_list(oxgen_list);
	pos = 0;
	while (oxgen_list != NULL && oxgen_list->next != NULL) {
		oxgen_list = filter_oxgen_list(oxgen_list, pos, nrbits);
		// printf("Pos %d:\n", pos);
		// print_list(oxgen_list);
		++pos;
	}
	if (oxgen_list == NULL)
		fprintf(stderr, "Unexpected 100%% filtering of oxgen list\n");
	else
		oxgen = oxgen_list->val;
	printf("oxgen: %d\n", oxgen);

	// filter co2scrub list
	// printf("Before:\n");
	// print_list(co2scrub_list);
	pos = 0;
	while (co2scrub_list != NULL && co2scrub_list->next != NULL) {
		co2scrub_list = filter_co2scrub_list(co2scrub_list, pos, nrbits);
		// printf("Pos %d:\n", pos);
		// print_list(co2scrub_list);
		++pos;
	}
	if (co2scrub_list == NULL)
		fprintf(stderr, "Unexpected 100%% filtering of co2scrub list\n");
	else
		co2scrub = co2scrub_list->val;
	printf("co2scrub: %d\n", co2scrub);

	printf("%d\n", oxgen * co2scrub);

	// clean up
	free(oxgen_list); // by design only 1 element left
	free(co2scrub_list); // by design only 1 element left
	return 0;
}
