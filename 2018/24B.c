#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// #define DEBUG

#define MAX_NAMES 128
#define MAX_GROUPS 64

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

struct group {
	int army_id; // which army this group belongs to
#ifdef DEBUG
	int group_id; // group in army
#endif

	int units;
	int hp;
	int weakto;   // bitfield
	int immuneto; // bitfield
	int attack_type;
	int attack;
	int initiative;

	struct group* target;
	struct group* attacked_by;
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

int cmp_by_effective_power(const void* a, const void* b) {
	struct group* ga = *(struct group**)a;
	struct group* gb = *(struct group**)b;
	int epa = ga->units * ga->attack;
	int epb = gb->units * gb->attack;
	return (epa == epb) ? gb->initiative - ga->initiative : epb - epa;
}

int cmp_by_initiative(const void* a, const void* b) {
	struct group* ga = *(struct group**)a;
	struct group* gb = *(struct group**)b;
	return ((gb->units == 0) ? 0 : gb->initiative) - ((ga->units == 0) ? 0 : ga->initiative);
}

int calc_damage(struct group* attacker, struct group* defender) { // not taking into account defender #units
	int attack_type_bf = (1 << attacker->attack_type);
	bool immuneto = (defender->immuneto & attack_type_bf) != 0;
	bool weakto = (defender->weakto & attack_type_bf) != 0;
	int effpow = attacker->units * attacker->attack;
	if (weakto)
		effpow *= 2;
	if (immuneto)
		effpow *= 0;
	return effpow;
}

struct group* choose_target(struct group* groups, int groups_sz, struct group* attacker) {
	int max_damage = 0;
	struct group* target = NULL;
	for (int ii = 0; ii < groups_sz; ++ii) {
		struct group* t = &groups[ii];
		if (attacker->army_id != t->army_id && t->attacked_by == NULL && t->units > 0) {
			int d = calc_damage(attacker, t);
#ifdef DEBUG
			if (d > 0)
				printf("Army/grp: %d/%d would deal Army/grp %d/%d %d damage\n",
						attacker->army_id + 1, attacker->group_id + 1, t->army_id + 1, t->group_id + 1, d);
#endif
			bool select = false;
			if (d > 0 && d >= max_damage) {
				if (d == max_damage) { // we need to break tie
					int ep_target = target->units * target->attack;
					int ep_t = t->units * t->attack;
					select = (ep_t > ep_target) || (ep_t == ep_target && t->initiative > target->initiative);
				}
				else
					select = true;
			}
			if (select) {
				max_damage = d;
				target = t;
			}
		}
	}
	return target;
}

void run_target_selection(struct group* groups, int groups_sz) {
	// reset
	for (int ii = 0; ii < groups_sz; ++ii) {
		groups[ii].target = NULL;
		groups[ii].attacked_by = NULL;
	}

	// sort by effective power (descending)
	struct group* sorted_by_pow[MAX_GROUPS];
	for (int ii = 0; ii < groups_sz; ++ii)
		sorted_by_pow[ii] = &groups[ii];
	qsort(sorted_by_pow, groups_sz, sizeof(struct group*), cmp_by_effective_power);

	for (int ii = 0; ii < groups_sz; ++ii) {
		if (sorted_by_pow[ii]->units > 0) {
			struct group* target = choose_target(groups, groups_sz, sorted_by_pow[ii]);
			if (target) {
				sorted_by_pow[ii]->target = target;
				target->attacked_by = sorted_by_pow[ii];
#ifdef DEBUG
	printf("Attacker army/grp %d/%d chose target %d/%d\n",
			sorted_by_pow[ii]->army_id + 1, sorted_by_pow[ii]->group_id + 1, target->army_id + 1, target->group_id + 1);
#endif
			}
		}
	}
}

int run_attack(struct group* groups, int groups_sz) {
	// returns total units killed (for deadlock detection)

	// sort by initiative (descending)
	struct group* sorted_by_initiative[MAX_GROUPS];
	for (int ii = 0; ii < groups_sz; ++ii)
		sorted_by_initiative[ii] = &groups[ii];
	qsort(sorted_by_initiative, groups_sz, sizeof(struct group*), cmp_by_initiative);

	int total_killed = 0;

	for (int ii = 0; ii < groups_sz; ++ii) {
		if (sorted_by_initiative[ii]->units > 0) {
			struct group* target = sorted_by_initiative[ii]->target;
			if (target) {
				int d = calc_damage(sorted_by_initiative[ii], target);
				int units_killed = d / target->hp;
				units_killed = MIN(units_killed, target->units);
#ifdef DEBUG
				printf("Army/grp %d/%d attacks Army/grp %d/%d and kills %d units\n",
						sorted_by_initiative[ii]->army_id + 1, sorted_by_initiative[ii]->group_id + 1,
						target->army_id + 1, target->group_id + 1,
						units_killed);
#endif
				target->units -= units_killed;
				total_killed += units_killed;
			}
		}
	}
	return total_killed;
}

bool army_has_units(struct group* groups, int groups_sz, int army_id) {
	for (int ii = 0; ii < groups_sz; ++ii)
		if (groups[ii].army_id == army_id && groups[ii].units > 0)
			return true;
	return false;
}


void show_army(struct group* groups, int groups_sz, int army_id, char* names[]) {
	for (int ii = 0; ii < groups_sz; ++ii) {
		if (groups[ii].army_id == army_id && groups[ii].units > 0) {
#ifdef DEBUG
			printf("Army/group: %d/%d; ", groups[ii].army_id + 1, groups[ii].group_id + 1);
#endif
			printf("%d units; %d hp; immune: ", groups[ii].units, groups[ii].hp);
			int bf = groups[ii].immuneto;
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
			bf = groups[ii].weakto;
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
			printf("; %d %s attack; initiative: %d", groups[ii].attack, names[groups[ii].attack_type], groups[ii].initiative);
			printf(" (eff. pow: %d)\n", groups[ii].units * groups[ii].attack);
		}
	}
	printf("\n");
}

int fight(struct group* groups, int groups_sz, int army_id, int boost) {
	// returns pos value if `army_id` wins. This army gets attack boost
	struct group groups_cpy[MAX_GROUPS];
	memcpy(groups_cpy, groups, groups_sz * sizeof(struct group));
	for (int ii = 0; ii < groups_sz; ++ii)
		groups_cpy[ii].attack += (groups[ii].army_id == army_id) ? boost : 0;

	bool fight_over = false;
	int killed = 0;
	while (!fight_over) {
		run_target_selection(groups_cpy, groups_sz);
		killed = run_attack(groups_cpy, groups_sz);
		fight_over = killed == 0 || !army_has_units(groups_cpy, groups_sz, 0) || !army_has_units(groups_cpy,groups_sz, 1);
	}
	if (killed == 0) // it's a tie due to deadlock
		return 0;
	// Count units
	int units = 0;
	for (int ii = 0; ii < groups_sz; ++ii)
		units += groups_cpy[ii].units * (groups_cpy[ii].army_id == army_id ? 1 : -1);
	return units;
}

int main(int argc, char* argv[]) {
	char* names[MAX_NAMES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		names[ii] = NULL; // put all terminations in place

	struct group groups[MAX_GROUPS];
	int groups_sz = 0;

	int army_id = 0;
#ifdef DEBUG
	int group_id[2] = {0, 0};
#endif

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		if (!strncmp(line, "Immune", 6))
			army_id = 0;
		else if (!strncmp(line, "Infection", 9))
			army_id = 1;
		else {
			char* l = line;
			groups[groups_sz].army_id = army_id;
#ifdef DEBUG
			groups[groups_sz].group_id = group_id[army_id]++;
#endif
			// read all numbers
			read_int(&l, &groups[groups_sz].units);
			read_int(&l, &groups[groups_sz].hp);
			read_int(&l, &groups[groups_sz].attack);
			read_int(&l, &groups[groups_sz].initiative);
			// weakness
			groups[groups_sz].weakto = 0;
			if ( (l = strstr(line, "weak to ")) ) {
				l += 8;
				groups[groups_sz].weakto = wordlist_to_bitfield(&l, names);
			}
			// immunity
			groups[groups_sz].immuneto = 0;
			if ( (l = strstr(line, "immune to ")) ) {
				l += 10;
				groups[groups_sz].immuneto = wordlist_to_bitfield(&l, names);
			}
			// attack type
			char word[64];
			l = strstr(line, "damage ") - 1;
			while (*(l-1) != ' ')
				--l;
			read_word(&l, word);
			groups[groups_sz].attack_type = name_to_idx(word, names);
			++groups_sz;
		}
	}
	free(line);

	// find search range itself
	int low = 0;
	int high = 1;
	while (fight(groups, groups_sz, 0, high) <= 0) {
		low = high;
		high *= 2;
	}

	// binary search in in range low ... high
	while (low < high - 1) {
		int mid = (low + high) / 2;
		int remain = fight(groups, groups_sz, 0, mid);
		if (remain <= 0)
			low = mid;
		else
			high = mid;
	}
	int remain = fight(groups, groups_sz, 0, high);
	printf("%d\n", remain);

	// clean up
	for (int ii = 0; names[ii] != NULL; ++ii)
		free(names[ii]);
	return 0;
}
