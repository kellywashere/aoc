#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// encoding state in a number s:
// groups of 4 bits indicating gen (2 MSB of group) and
// chip floor (2 LSB of group).
// then 2 MSBs of s are elevator location
// input file shows 5 elements, so we have 5 * 4 + 2 bits
// 22 bit nr for state envoding (4 million states)
// of course here floors are labeled 0..3 and not 1..4
// target state: all bits set
// invalid state: 1 or more unpowered chips on a floor with incompatible gen

bool safe_state(uint32_t s, int nr_el) {
	// state UNsafe when both:
	// 1. a chip is on a different floor than its generator
	// 2. there is another generator on the same floor as that chip
	for (int e = 0; e < nr_el; ++e) {
		int gen_chip = (s >> (4 * e)) & 0x0f;
		int chipfloor = gen_chip & 0x03;
		int genfloor = gen_chip >> 2;
		if (chipfloor != genfloor) { // unpowered chip
			// check if different gen on chipfloor
			for (int g = 0; g < nr_el; ++g) {
				if (g == e)
					continue;
				genfloor = (s >> (4 * g + 2)) & 0x03;
				if (genfloor == chipfloor)
					return false; // chip is fried !
			}
		}
	}
	return true; // safe
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
	}
	free(line);
	return 0;
}
