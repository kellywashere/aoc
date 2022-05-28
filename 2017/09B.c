#include <stdio.h>

#define STACKSIZE 128

int main(int argc, char* argv[]) {
	char brackstack[STACKSIZE];
	int sp = 0; // stack pointer == depth
	int garbagecnt = 0;

	int c = getc(stdin);
	while (c != EOF) {
		if (c == '!') {
			getc(stdin); // skip next
		}
		else if (sp > 0 && brackstack[sp - 1] == '<') { // we are currently in garbage
			if (c == '>')
				--sp; // exit garbage
			else
				++garbagecnt;
		}
		else if (c == '<' || c == '{') {
			brackstack[sp++] = c;
			if (sp >= STACKSIZE) {
				fprintf(stderr, "Stack size insufficient! Exiting...\n");
				return 1;
			}
		}
		else if (c == '}' && sp > 0) {
			--sp;
			if (brackstack[sp] != '{')
				fprintf(stderr, "Unexpected group close }\n");
		}
		c = getc(stdin);
	}
	printf("%d\n", garbagecnt);
	return 0;
}
