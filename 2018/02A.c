#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/*
int cmp_char(const void* a, const void* b) {
	return *(char*)a - *(char*)b;
}
*/

int idlen(char* l) {
	char* ll = l;
	while (*ll && !isspace(*ll))
		++ll;
	return (int)(ll - l);
}

void has2has3(char* l, bool* has2, bool* has3) {
	int count[26] = {0};
	int len = idlen(l);
	*has2 = false;
	*has3 = false;
	for (int ii = 0; ii < len; ++ii)
		++count[l[ii]-'a'];
	for (int ii = 0; ii < 26; ++ii) {
		*has2 = *has2 || count[ii] == 2;
		*has3 = *has3 || count[ii] == 3;
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int has2count = 0;
	int has3count = 0;
	while (getline(&line, &len, stdin) != -1) {
		bool has2, has3;
		has2has3(line, &has2, &has3);
		has2count += has2 ? 1 : 0;
		has3count += has3 ? 1 : 0;
	}
	printf("%d\n", has2count * has3count);
	free(line);
	return 0;
}
