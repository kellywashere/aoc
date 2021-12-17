#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LITERAL_VALUE 4

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

#define is_hexdigit(c) (((c)>='0' && (c)<'9') || ((c)>='A' && (c)<='F'))

struct binstr {
	char* str;
	int   len;
	int   idx;
};

struct binstr* hex_to_binstr(const char* hexstr) {
	int hexlen = 0;
	while (is_hexdigit(hexstr[hexlen]))
		++hexlen;
	struct binstr* binstr = mymalloc(sizeof(struct binstr));
	binstr->str = mymalloc((4 * hexlen + 1) * sizeof(char));
	int ii, jj;
	for (ii = 0; ii < hexlen; ++ii) {
		int hd;
		if (hexstr[ii] >= '0' && hexstr[ii] <= '9')
			hd = hexstr[ii] - '0';
		else
			hd = hexstr[ii] - 'A' + 10;
		for (jj = 0; jj < 4; ++jj)
			binstr->str[4 * ii + jj] = '0' + ((hd >> (3 - jj)) & 1);
	}
	binstr->str[hexlen * 4] = '\0';
	binstr->len = 4 * hexlen;
	binstr->idx = 0;
	return binstr;
}

int read_bits(struct binstr* bstr, int len) {
	int x = 0;
	for (; len; --len) {
		x = (x << 1) + (bstr->str[bstr->idx++] - '0');
	}
	return x;
}

int read_literal_value(struct binstr* bstr) {
	int litval = 0;
	int b;
	do {
		b = read_bits(bstr, 5);
		litval = (litval << 4) + (b & 0x0F);
	} while ( (b & 0x10) );
	// make sure binstr idx is mult of 4
	bstr->idx = ((bstr->idx + 3) / 4) * 4;
	return litval;
}

struct operator {
	int lengthtypeID;
	int length;
};

union content {
	int             literal;
	struct operator operator;
};

struct packet {
	int           version;
	int           typeID;
	union content content;
};

void read_packet(struct binstr* bstr, struct packet* packet) {
	packet->version = read_bits(bstr, 3);
	packet->typeID = read_bits(bstr, 3);
	if (packet->typeID == LITERAL_VALUE) {
		packet->content.literal = read_literal_value(bstr);
	}
	else { // operator packet
		int lengthtypeID = read_bits(bstr, 1);
		packet->content.operator.lengthtypeID = lengthtypeID;
		if (lengthtypeID == 0)
			packet->content.operator.length = read_bits(bstr, 15);
		else
			packet->content.operator.length = read_bits(bstr, 11);
	}
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	struct binstr* binstr = hex_to_binstr(line);
	free(line);

	printf("%s\n", binstr->str);
	struct packet packet;
	read_packet(binstr, &packet);
	printf("version: %d\n", packet.version);
	printf("typeID:  %d\n", packet.typeID);
	if (packet.typeID == LITERAL_VALUE)
		printf("Literal value: %d\n", packet.content.literal);

	// clean up
	free(binstr->str);
	free(binstr);
	return 0;
}
