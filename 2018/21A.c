#include <stdio.h>
#include <inttypes.h>

// see notes21.txt for reverse engineering of ASM code

int main(void) {
	// we stop at the end if r0 == r1, so we choose r0 to equal r1 as it appears after first cycle
	uint64_t r1 = 0;
	uint64_t r4 = 0;

	r4 = r1 | 0x10000;

	r1 = 0x39F737;
	do {
		r1 += (r4 & 0xFF);
		r1 &= 0xFFFFFF;
		r1 *= 65899;
		// r1 = ((r1 & 0xFF) << 16) + ((r1 & 0xFFFF) << 8) + r1 * 0x6b;
		r1 &= 0xFFFFFF;

		r4 >>= 8;
	} while (r4);

	printf("%" PRIu64 "\n", r1);
	return 0;
}
