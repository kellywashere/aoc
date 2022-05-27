#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct int_array {
	int* data;
	int  size;
	int  capacity;
};

struct int_array* create_int_array() {
	struct int_array* a = malloc(sizeof(struct int_array));
	a->size = 0;
	a->capacity = 256;
	a->data = malloc(a->capacity * sizeof(int));
	return a;
}

void destroy_int_array(struct int_array* a) {
	if (a) {
		free(a->data);
		free(a);
	}
}

void int_array_add(struct int_array* a, int x) {
	if (a->size + 1 >= a->capacity) {
		a->capacity *= 2;
		a->data = realloc(a->data, a->capacity * sizeof(int));
	}
	a->data[a->size++] = x;
}

int simulate(struct int_array* jmps) {
	int steps = 0;
	int pc = 0;
	while (pc >= 0 && pc < jmps->size) {
		int j = jmps->data[pc];
		jmps->data[pc] = j >= 3 ? j - 1 : j + 1;
		pc = pc + j;
		++steps;
	}
	return steps;
}

int main(int argc, char* argv[]) {
	int x;
	struct int_array* arr = create_int_array();
	while (scanf("%d", &x) == 1)
		int_array_add(arr, x);
	int steps = simulate(arr);
	printf("%d\n", steps);
	destroy_int_array(arr);
	return 0;
}
