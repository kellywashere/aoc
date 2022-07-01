#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAMES 128
#define MAX_GROUPS 64

struct group {
	int units;
	int hp;
	int weakto;   // bitfield
	int immuneto; // bitfield
	int damage_type;
	int damage;
	int initiative;

	int target;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

bool read_word(char** pLine, char* word) {
	// skips characters not in [a-z] before word automatically
	char* line = *pLine;
	int len = 0;
	while (*line && !(*line >= 'a' && *line <= 'z'))
		++line;
	while (*line >= 'a' && *line <= 'z') {
		word[len++] = *line;
		++line;
	}
	if (len > 0)
		word[len] = '\0';
	*pLine = line;
	return len > 0;
}

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

int name_to_idx(char* name, char* names[]) {
	// if name not in names, adds it
	int ii;
	for (ii = 0; names[ii] != NULL; ++ii)
		if (!strcmp(names[ii], name))
			return ii;
	int l = strlen(name);
	names[ii] = malloc((l + 1) * sizeof(char));
	strcpy(names[ii], name);
	return ii;
}


int wordlist_to_bitfield(char **pLine, char* names[]) {
	int bf = 0;
	char* l = *pLine;
	char word[64];

	bool more_words = true;
	while (more_words) {
		read_word(&l, word);
		int word_idx = name_to_idx(word, names);
		bf |= (1 << word_idx);
		more_words = *l == ',';
		l += more_words ? 2 : 0;
	}
	*pLine = l;
	return bf;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place

	struct group army[2][MAX_GROUPS];
	int nr_groups[2] = {0, 0}; // groups in each army
	int army_idx = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		if (!strncmp(line, "Immune", 6))
			army_idx = 0;
		else if (!strncmp(line, "Infection", 9))
			army_idx = 1;
		else {
			int idx = nr_groups[army_idx];
			char* l = line;
			// read all numbers
			read_int(&l, &army[army_idx][idx].units);
			read_int(&l, &army[army_idx][idx].hp);
			read_int(&l, &army[army_idx][idx].damage);
			read_int(&l, &army[army_idx][idx].initiative);
			// weakness
			army[army_idx][idx].weakto = 0;
			if ( (l = strstr(line, "weak to ")) ) {
				l += 8;
				army[army_idx][idx].weakto = wordlist_to_bitfield(&l, names);
			}
			// immunity
			army[army_idx][idx].immuneto = 0;
			if ( (l = strstr(line, "immune to ")) ) {
				l += 10;
				army[army_idx][idx].immuneto = wordlist_to_bitfield(&l, names);
			}
			// damage type
			char word[64];
			l = strstr(line, "damage ") - 1;
			while (*(l-1) != ' ')
				--l;
			read_word(&l, word);
			army[army_idx][idx].damage_type = name_to_idx(word, names);
			++nr_groups[army_idx];
		}
	}
	free(line);

	for (int army_idx = 0; army_idx < 2; ++army_idx) {
		// 2374 units each with 41150 hit points (immune to bludgeoning, slashing, radiation; weak to cold) with an attack that does 34 bludgeoning damage at initiative 6
		for (int ii = 0; ii < nr_groups[army_idx]; ++ii) {
			printf("%d units; %d hp; immune: ", army[army_idx][ii].units, army[army_idx][ii].hp);
			int bf = army[army_idx][ii].immuneto;
			int idx = 0;
			while (bf) {
				if ((bf & 1) == 1) {
					printf("%s", names[idx]);
					if (bf > 1)
						printf(", ");
				}
				++idx;
				bf >>= 1;
			}
			printf("; weak to: ");
			bf = army[army_idx][ii].weakto;
			idx = 0;
			while (bf) {
				if ((bf & 1) == 1) {
					printf("%s", names[idx]);
					if (bf > 1)
						printf(", ");
				}
				++idx;
				bf >>= 1;
			}
			printf("; %d %s damage; initiative: %d\n", army[army_idx][ii].damage, names[army[army_idx][ii].damage_type], army[army_idx][ii].initiative);
		}
		printf("\n");
	}

	// clean up
	for (int ii = 0; names[ii] != NULL; ++ii)
		free(names[ii]);
	return 0;
}
