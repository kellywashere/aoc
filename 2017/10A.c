#include <stdio.h>
#include <stdlib.h>

void reverse_list(int* list, int listsize, int pos, int len) {
	for (int ii = 0; ii < len / 2; ++ii) {
		int idx1 = (pos + ii) % listsize;
		int idx2 = (pos + len - ii - 1) % listsize;
		int v = list[idx1];
		list[idx1] = list[idx2];
		list[idx2] = v;
	}
}

int main(int argc, char* argv[]) {
	int list[256];
	for (int ii = 0; ii < 256; ++ii)
		list[ii] = ii;
	int listsize = 256;
	if (argc > 1)
		listsize = atoi(argv[1]);
	int pos = 0;
	int skipsize = 0;

	int len;
	while (scanf("%d%*c", &len) == 1) {
		reverse_list(list, listsize, pos, len);
		pos = (pos + len + skipsize) % listsize;
		++skipsize;
	}
	/*
	for (int ii = 0; ii < listsize; ++ii)
		printf("%4d ", list[ii]);
	printf("\n");
	*/
	printf("%d\n", list[0] * list[1]);
	return 0;
}
