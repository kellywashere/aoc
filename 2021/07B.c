#include <stdio.h>
#include <stdlib.h>


void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "Could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

struct int_array {
	int*   values;
	size_t size;
	size_t capacity;
};

struct int_array* create_int_array(int cap) {
	struct int_array* array = mymalloc(sizeof(struct int_array));
	array->values = mymalloc(cap * sizeof(int));
	array->size = 0;
	array->capacity = cap;
}

void destroy_int_array(struct int_array* array) {
	if (array)
		free(array->values);
	free(array);
}

void int_array_append(struct int_array* array, int x) {
	if (array->size == array->capacity) {
		array->capacity *= 2;
		array->values = myrealloc(array->values, array->capacity * sizeof(int));
	}
	array->values[array->size++] = x;
}

int cmp_int_array_vals(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

void int_array_sort(struct int_array* array) {
	qsort(array->values, array->size, sizeof(int), cmp_int_array_vals);
}

int calc_fuel(int p[], int len, int t) {
	int ii;
	int fuel = 0;
	for (ii = 0; ii < len; ++ii) {
		int dist = abs(t - p[ii]);
		fuel += dist * (dist + 1) / 2;
	}
	return fuel;
}
int main(int argc, char* argv[]) {
	int ii;
	struct int_array* positions = create_int_array(256);

	// get 1 line from stdin
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);

	// process line -> positions
	char* s = line;
	while (*s) {
		int pos = strtol(s, &s, 10);
		if (*s == ',' || *s == '\n')
			++s;
		int_array_append(positions, pos);
	}
	int_array_sort(positions);
	// ugly brute force:
	int pos = positions->values[0];
	int fuel = calc_fuel(positions->values, positions->size, pos);
	int next_fuel = calc_fuel(positions->values, positions->size, pos + 1);
	while (next_fuel < fuel) {
		++pos;
		fuel = next_fuel;
		next_fuel = calc_fuel(positions->values, positions->size, pos + 1);
	}
	printf("pos: %d, fuel: %d\n", pos, fuel);
	printf("%d\n", fuel);

	free(line);
	return 0;
}
