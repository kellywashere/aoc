#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// segments are mapped to bit positions in 7 bit nr
//  0000
// 1    2
// 1    2
//  3333
// 4    5
// 4    5
//  6666
int dig2seg[10] = {0x77, 0x24, 0x5D, 0x6D, 0x2E, 0x6B, 0x7B, 0x25, 0x7F, 0x6F};

/*
void print_horz_segment(int s) {
	printf(s ? " ****\n" : "\n");
}

void print_vert_segment(int s) {
	for (int ii = 0; ii < 2; ++ii) {
		switch (s & 0x03) {
			case 1:
				printf("*\n");
				break;
			case 2:
				printf("     *\n");
				break;
			case 3:
				printf("*    *\n");
				break;
			default:
				printf("\n");
				break;
		}
	}
}

void print_segment_digit(int x) {
	int s = dig2seg[x];
	print_horz_segment(s & 1);
	print_vert_segment(s >> 1);
	print_horz_segment(s & 8);
	print_vert_segment(s >> 4);
	print_horz_segment(s & 0x40);
}
*/

// From projecteuler, problem 041.c
bool next_perm(int a[], int len) {
	// returns false when no next permutation exists
	// step 1: from right to left, find first element higher than its right neighbor: pivot = a[idxPivot]
	// step 2: sort elements to right of idxPivot (invert order in this case!)
	// step 3: from idxPivot upward, find first element > pivot: idxSwap
	// step 4: swap pivot with a[idxSwap]
	int idxPivot;
	for (idxPivot = len - 2; idxPivot >= 0; --idxPivot) {
		if (a[idxPivot] < a[idxPivot + 1]) {
			// invert order of elements idxPivot+1 ... len-1
			int t;
			int ii = idxPivot + 1;
			int jj = len - 1;
			while (jj > ii) {
				t = a[jj];
				a[jj--] = a[ii];
				a[ii++] = t;
			}
			// from idxPivot upward, find first element > pivot: ii
			for (ii = idxPivot + 1; a[ii] < a[idxPivot]; ++ii);
			// swap a[ii] with a[idxPivot]
			t = a[ii];
			a[ii] = a[idxPivot];
			a[idxPivot] = t;
			break;
		}
	}
	return (idxPivot >= 0);
}

int str2wire(char* str) { // converts input str to active wire bitfield
	int w = 0;
	while (*str >= 'a' && *str <= 'g') {
		int b = *str - 'a';
		w |= (1 << b);
		++str;
	}
	return w;
}

int wire2seg(int w, int connections[]) {
	// connections[n] --> bit n in w activates bit connections[n] in output
	int seg = 0;
	for (int n = 0; n < 7; ++n) {
		if ( (w & (1 << n)) )
			seg |= (1 << connections[n]);
	}
	return seg;
}

int seg2digit(int s) { // returns -1 if invalid
	for (int d = 0; d <= 9; ++d)
		if (dig2seg[d] == s)
			return d;
	return -1;
}

#define MAX_IN 25
#define MAX_OUT 10
int process_line(char* line) {
	int val;
	int ii;
	int input[MAX_IN];
	int len_in = 0;
	int output[MAX_OUT];
	int len_out = 0;
	char* delim = " \t\n";

	char* t = strtok(line, delim);
	while (t[0] != '|') {
		input[len_in++] = str2wire(t);
		t = strtok(NULL, delim);
	}
	t = strtok(NULL, delim);
	while (t) {
		output[len_out++] = str2wire(t);
		t = strtok(NULL, delim);
	}

	int connections[7]; // mapping wires to segments
	// brute force all options
	for (ii = 0; ii < 7; ++ii)
		connections[ii] = ii;
	do {
		// try permutation
		val = 0;
		bool valid = true;
		for (ii = 0; valid && ii < len_in; ++ii) {
			int seg = wire2seg(input[ii], connections);
			valid = valid && seg2digit(seg) != -1;
		}
		if (valid) { // decode output
			for (ii = 0; ii < len_out; ++ii) {
				int seg = wire2seg(output[ii], connections);
				int d = seg2digit(seg);
				if (d == -1) {
					val = 0;
					break;
				}
				val = 10 * val + d;
			}
		}
	} while (val == 0 && next_perm(connections, 7));

	return val;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		count += process_line(line);
	}
	printf("%d\n", count);
	free(line);
	return 0;
}
