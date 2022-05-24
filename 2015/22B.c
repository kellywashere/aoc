#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

int read_int_from_stdin() {
	char c;
	int x = 0;
	c = getc(stdin);
	while (c != EOF && (c < '0' || c > '9'))
		c = getc(stdin);
	while (c >= '0' && c <= '9') {
		x = 10 * x + c - '0';
		c = getc(stdin);
	}
	return x;
}

#define MIN(a,b) ((a)<(b)?(a):(b))

#define MAGICMISSILE_COST  53
#define DRAIN_COST         73
#define SHIELD_COST       113
#define POISON_COST       173
#define RECHARGE_COST     229

#define CHEAPEST_SPELL     53

#define SHIELD_DURATION     6
#define POISON_DURATION     6
#define RECHARGE_DURATION   5

#define MAGICMISSILE_DAMAGE 4
#define DRAIN_DAMAGE        2
#define DRAIN_HEAL          2
#define SHIELD_ARMOR        7
#define POISON_DAMAGE       3
#define RECHARGE_MANA     101

enum spell {
	MAGICMISSILE = 0,
	DRAIN,
	SHIELD,
	POISON,
	RECHARGE,

	NR_SPELLS
};

enum outcome {
	BOSS_WINS = -1,
	NO_WINNER = 0, // when game is ongoing
	PLAYER_WINS = 1
};

int spell_cost[]      = {MAGICMISSILE_COST, DRAIN_COST, SHIELD_COST, POISON_COST, RECHARGE_COST};
int effect_duration[] = {0, 0, SHIELD_DURATION, POISON_DURATION, RECHARGE_DURATION};

struct gamestate {
	// int turn;             // even: player, odd: boss
	int player_hit;       // player's hit points
	int boss_hit;         // boss's hit points
	int boss_attack;      // (constant) boss's attack
	int mana;             // player's mana
	int effect_count[NR_SPELLS]; // all effect counters (also for spells that do not need one, for code simplity)

	int mana_spent;       // mana spent so far
};

bool can_cast_spell(struct gamestate* gs, enum spell spell) {
	return (gs->mana >= spell_cost[spell] && gs->effect_count[spell] == 0);
}

void cast_spell(struct gamestate* gs, enum spell spell) {
	gs->mana -= spell_cost[spell];
	gs->mana_spent += spell_cost[spell];
	gs->effect_count[spell] = effect_duration[spell];
	switch (spell) {
		case MAGICMISSILE:
			gs->boss_hit -= MAGICMISSILE_DAMAGE;
			break;
		case DRAIN:
			gs->boss_hit -= DRAIN_DAMAGE;
			gs->player_hit += DRAIN_HEAL;
			break;
		default:
			break;
	}
}

void do_effects(struct gamestate* gs) {
	// note: shield handled in boss_attack !
	if (gs->effect_count[POISON])
		gs->boss_hit -= POISON_DAMAGE;
	if (gs->effect_count[RECHARGE])
		gs->mana += RECHARGE_MANA;
	// update counters
	for (int ii = 0; ii < NR_SPELLS; ++ii)
		if (gs->effect_count[ii])
			--gs->effect_count[ii];
}

void boss_attack(struct gamestate* gs) {
	int player_armor = gs->effect_count[SHIELD] ? 7 : 0;
	gs->player_hit -= gs->boss_attack - player_armor;
}

int find_lowest_cost_path(struct gamestate* gs, int lowest_so_far) {
	if (gs->mana_spent >= lowest_so_far) // No need to go this path: prune
		return lowest_so_far;

	// Player's turn first
	// 22B: player loses 1 hit point at start of turn
	--gs->player_hit;
	if (gs->player_hit <= 0) // boss wins
		return lowest_so_far;
	
	do_effects(gs);
	if (gs->boss_hit <= 0) // player wins
		return MIN(gs->mana_spent, lowest_so_far); // return gs->mana_spent also works, MIN is clearer
	if (gs->mana < CHEAPEST_SPELL) // boss wins
		return lowest_so_far;
	// game still on: try all spells
	for (enum spell spell = MAGICMISSILE; spell < NR_SPELLS; ++spell) {
		if (!can_cast_spell(gs, spell))
			continue;
		struct gamestate gs_cpy = *gs; // need a copy for undoing in the loop
		cast_spell(&gs_cpy, spell);
		if (gs_cpy.boss_hit <= 0) // player wins
			lowest_so_far = MIN(gs_cpy.mana_spent, lowest_so_far);
		else { // boss gets turn
			do_effects(&gs_cpy);
			if (gs_cpy.boss_hit <= 0) // player wins
				lowest_so_far = MIN(gs_cpy.mana_spent, lowest_so_far);
			else { // game still on: boss attacks
				boss_attack(&gs_cpy);
				if (gs_cpy.player_hit > 0) { // recurse
					int cost = find_lowest_cost_path(&gs_cpy, lowest_so_far);
					lowest_so_far = MIN(cost, lowest_so_far);
				}
			}
		}
	}
	return lowest_so_far;
}

int main(int argc, char* argv[]) {
	struct gamestate gs;
	gs.player_hit = 50;
	gs.mana = 500;
	gs.mana_spent = 0;
	for (enum spell spell = 0; spell < NR_SPELLS; ++spell)
		gs.effect_count[spell] = 0;
	gs.boss_hit = read_int_from_stdin();
	gs.boss_attack = read_int_from_stdin();

	int lowest_cost = find_lowest_cost_path(&gs, INT_MAX);
	printf("%d\n", lowest_cost);

	return 0;
}

