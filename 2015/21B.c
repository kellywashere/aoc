#include <stdio.h>

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

struct weapon {
	int cost;
	int attack; // "damage"
};

struct armor {
	int cost;
	int armor;
};

struct ring {
	int cost;
	int attack;
	int armor;
};

struct fighter {
	int hitpoints;
	int attack;
	int armor;
};

struct weapon weapons[] = {{8,4}, {10,5}, {25,6}, {40,7}, {74,8}};
struct armor  armor[]   = {{0,0}, {13,1}, {31,2}, {53,3}, {75,4}, {102,5}}; // added 0-armor for ease of search algo
struct ring   rings[]   = {{0,0,0}, {25,1,0}, {50,2,0}, {100,3,0}, {20,0,1}, {40,0,2}, {80,0,3}}; // added 0-ring

int calc_turns(struct fighter* attacker, struct fighter* victim) {
	int damage = attacker->attack - victim->armor;
	damage = damage < 1 ? 1 : damage;
	// turns = ceil(victim->hitpoints / damage);
	return (victim->hitpoints + damage - 1) / damage;
}

int main(int argc, char* argv[]) {
	int nr_weapons = sizeof(weapons)/sizeof(weapons[0]);
	int nr_armor   = sizeof(armor)/sizeof(armor[0]);
	int nr_rings   = sizeof(rings)/sizeof(rings[0]);

	struct fighter player = {100, 0, 0};
	struct fighter boss;
	boss.hitpoints = read_int_from_stdin();
	boss.attack = read_int_from_stdin();
	boss.armor = read_int_from_stdin();

	int maxcost = 0;
	// brute force search
	for (int w = 0; w < nr_weapons; ++w) {
		for (int a = 0; a < nr_armor; ++a) {
			for (int ring1 = 0; ring1 < nr_rings; ++ring1) {
				for (int ring2 = 0; ring2 < nr_rings; ++ring2) {
					if (ring2 > 0 && ring2 == ring1)
						continue;
					player.attack = weapons[w].attack + rings[ring1].attack + rings[ring2].attack;
					player.armor = armor[a].armor + rings[ring1].armor + rings[ring2].armor;
					int totalcost = weapons[w].cost + armor[a].cost;
					totalcost += rings[ring1].cost + rings[ring2].cost;
					int turns_player = calc_turns(&player, &boss);
					int turns_boss   = calc_turns(&boss, &player);
					if (turns_player > turns_boss && totalcost > maxcost) { // player loses
						maxcost = totalcost;
					}
				}
			}
		}
	}
	printf("%d\n", maxcost);

	return 0;
}
