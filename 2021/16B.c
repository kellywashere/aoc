#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

enum {
	SUM = 0,
	PRODUCT,
	MINIMUM,
	MAXIMUM,
	LITERAL_VALUE,
	GREATER_THAN,
	LESS_THAN,
	EQUAL_TO
};

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

#define is_hexdigit(c) (((c)>='0' && (c)<='9') || ((c)>='A' && (c)<='F'))

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

uint64_t read_literal_value(struct binstr* bstr) {
	uint64_t litval = 0;
	int b;
	do {
		b = read_bits(bstr, 5);
		litval = (litval << 4) + (b & 0x0F);
	} while ( (b & 0x10) );
	return litval;
}

struct operator {
	int lengthtypeID;
	int length;
};

union content {
	uint64_t        literal;
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
	// printf("version: %d\n", packet->version);
	// printf("typeID:  %d\n", packet->typeID);
	if (packet->typeID != LITERAL_VALUE) { // operator packet
		int lengthtypeID = read_bits(bstr, 1);
		// printf("lengthtypeID: %d\n", lengthtypeID);
		packet->content.operator.lengthtypeID = lengthtypeID;
		if (lengthtypeID == 0) {
			packet->content.operator.length = read_bits(bstr, 15);
			// printf("length: %d bits\n", packet->content.operator.length);
		}
		else {
			packet->content.operator.length = read_bits(bstr, 11);
			// printf("length: %d packets\n", packet->content.operator.length);
		}
	}
}

uint64_t eval_packet(struct binstr* bstr, struct packet* packet);

int g_count = 0;

typedef uint64_t (*foldfn)(uint64_t, uint64_t); // e.g. acc = (*foldfn)(acc, newval)

uint64_t fold_packets(struct binstr* bstr, struct packet* packet, foldfn fn) {
	bool first_packet = true;
	uint64_t acc;
	struct packet p;
	int lengthtypeID = packet->content.operator.lengthtypeID;
	int length = packet->content.operator.length;
	while (length > 0) {
		int idx_cpy = bstr->idx;
		read_packet(bstr, &p);
		if (first_packet) {
			acc = eval_packet(bstr, &p);
			first_packet = false;
		}
		else {
			acc = (*fn)(acc, eval_packet(bstr, &p));
		}
		if (lengthtypeID == 0)
			length -= (bstr->idx - idx_cpy);
		else
			--length;
	}
	return acc;
}

uint64_t sum(uint64_t acc, uint64_t x) {
	return acc + x;
}

uint64_t product(uint64_t acc, uint64_t x) {
	return acc * x;
}

uint64_t minimum(uint64_t acc, uint64_t x) {
	return acc < x ? acc : x;
}

uint64_t maximum(uint64_t acc, uint64_t x) {
	return acc > x ? acc : x;
}

uint64_t gt(uint64_t acc, uint64_t x) {
	return acc > x ? 1 : 0;
}

uint64_t lt(uint64_t acc, uint64_t x) {
	return acc < x ? 1 : 0;
}

uint64_t eq(uint64_t acc, uint64_t x) {
	return acc == x ? 1 : 0;
}

uint64_t eval_packet(struct binstr* bstr, struct packet* packet) {
	// printf("Evaluating packet with type %d\n", packet->typeID);
	switch (packet->typeID) {
		case SUM:
			return fold_packets(bstr, packet, sum);
		case PRODUCT:
			return fold_packets(bstr, packet, product);
		case MINIMUM:
			return fold_packets(bstr, packet, minimum);
		case MAXIMUM:
			return fold_packets(bstr, packet, maximum);
		case LITERAL_VALUE:
			return read_literal_value(bstr);
		case GREATER_THAN:
			return fold_packets(bstr, packet, gt);
		case LESS_THAN:
			return fold_packets(bstr, packet, lt);
		case EQUAL_TO:
			return fold_packets(bstr, packet, eq);
		default:
			fprintf(stderr, "Not implemented: %d\n", packet->typeID);
	}
	return 666;
}

uint64_t eval_transmission(struct binstr* bstr) {
	struct packet packet;
	read_packet(bstr, &packet);
	return eval_packet(bstr, &packet);
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	struct binstr* binstr = hex_to_binstr(line);
	// printf("%s\n", binstr->str);
	free(line);

	printf("%" PRIu64 "\n", eval_transmission(binstr));

	// clean up
	free(binstr->str);
	free(binstr);
	return 0;
}
