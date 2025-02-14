#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define CEILDIV(x,y) (((x)+(y-1))/(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
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
	*pLine = line;
	return num;
}

enum mat {
	ORE = 0,
	CLAY,
	OBSIDIAN,
	GEODE
};

char* names[] = { "ore", "clay", "obsidian", "geode" };

bool can_build(enum mat b, int material[], int orecost[], int prevcost[]) {
	return (material[0] >= orecost[b] && (prevcost[b] == 0 || material[b - 1] >= prevcost[b]) );
}

void build_bot(enum mat b, int material[], int orecost[], int prevcost[]) {
	printf("Spend %d ore", orecost[b]);
	if (prevcost[b])
		printf(" and %d %s", prevcost[b], names[b - 1]);
	printf(" to start building a %s-bot.\n", names[b]);
	material[0] -= orecost[b];
	if (prevcost[b])
		material[b - 1] -= prevcost[b];
}

int calc_turns_to_build(enum mat b, int bots[], int material[], int orecost[], int prevcost[]) {
	int needed = orecost[b] - material[ORE];
	int turns_ore = needed <= 0 ? 0 : CEILDIV(needed, bots[ORE]);
	needed = prevcost[b] ? prevcost[b] - material[b - 1] : 0;
	int turns_other = needed <= 0 ? 0 :
		(bots[b - 1] == 0 ? INT_MAX : CEILDIV(needed, bots[b - 1]));
	return MAX(turns_ore, turns_other);
}

bool should_we_build(enum mat buildbot, enum mat target,
		int bots[], int material[], int orecost[], int prevcost[]) {
	// should we build buildbot?
	// ony if it leads to getting target bot faster
	// "faster" is evaluated simplisticly
	if (can_build(buildbot, material, orecost, prevcost)) {
		int turns_nobuild = calc_turns_to_build(target, bots, material, orecost, prevcost);
		// what if we build buildbot first?
		// pretend we build the bot, and wait this turn for it to finish
		material[ORE] -= orecost[buildbot];
		if (prevcost[buildbot])
			material[buildbot - 1] -= prevcost[buildbot];
		// pretend collection for 1 turn
		for (int ii = 0; ii < 4; ++ii)
			material[ii] += bots[ii];
		++bots[buildbot];
		int turns_build = 1 + calc_turns_to_build(target, bots, material, orecost, prevcost);
		// undo changes
		--bots[buildbot];
		for (int ii = 0; ii < 4; ++ii)
			material[ii] -= bots[ii];
		if (prevcost[buildbot])
			material[buildbot - 1] += prevcost[buildbot];
		material[ORE] += orecost[buildbot];
		printf("It would take %d turns to build %s-bot if we build %s-bot now\n", turns_build, names[target], names[buildbot]);
		printf("It would take %d turns to build %s-bot if we don't build %s-bot now\n", turns_nobuild, names[target], names[buildbot]);
		return turns_build <= turns_nobuild;
	}
	return false;
}

int exec_blueprint(char* line) {
	// strat: build geode-bot whenever possible
	// build obs-bot when it results in new geode-bot faster
	// build clay bot when it results in obs-bot faster
	// build ore-bot when it results in clay bot faster (?)
	// "faster" is evaluated in simplistic way here
	int bots[4]; // ore, clay, obsidian, geode
	int orecost[4]; // ore-cost for the bots
	int prevcost[4]; // misc cost for the bots
	int material[4]; // amount of meterials
	int build[4]; // amount of bots to build this turn
	bots[0] = 1;
	for (int ii = 1; ii < 4; ++ii)
		bots[ii] = 0;
	for (int ii = 0; ii < 4; ++ii)
		material[ii] = 0;
	orecost[0] = read_int(&line);
	prevcost[0] = 0;
	orecost[1] = read_int(&line);
	prevcost[1] = 0;
	orecost[2] = read_int(&line);
	prevcost[2] = read_int(&line);
	orecost[3] = read_int(&line);
	prevcost[3] = read_int(&line);

	printf("Blueprint:\n");
	printf("  Each ore robot costs %d ore.\n", orecost[0]);
	printf("  Each clay robot costs %d ore.\n", orecost[1]);
	printf("  Each obsidian robot costs %d ore and %d clay.\n", orecost[2], prevcost[2]);
	printf("  Each geode robot costs %d ore and %d obsidian.\n", orecost[3], prevcost[3]);

	for (int turn = 1; turn <= 24; ++turn) {
		printf("\n== Minute %d ==\n", turn);


		printf("Starting materials: %d ore, %d clay, %d obs, %d geo\n", material[0], material[1], material[2], material[3]);
		printf("Starting bots:      %d ore, %d clay, %d obs, %d geo\n", bots[0], bots[1], bots[2], bots[3]);




		for (int ii = 0; ii < 4; ++ii)
			build[ii] = 0;
		// build strategy:


		while (can_build(ORE, material, orecost, prevcost) && bots[ORE] < 1) {
			build_bot(ORE, material, orecost, prevcost);
			++build[ORE];
		}







		while (can_build(GEODE, material, orecost, prevcost)) {
			build_bot(GEODE, material, orecost, prevcost);
			++build[GEODE];
		}
		while (should_we_build(OBSIDIAN, GEODE, bots, material, orecost, prevcost)) {
			build_bot(OBSIDIAN, material, orecost, prevcost);
			++build[OBSIDIAN];
		}
		while (should_we_build(CLAY, OBSIDIAN, bots, material, orecost, prevcost)) {
			build_bot(CLAY, material, orecost, prevcost);
			++build[CLAY];
		}
		/*
		while (should_we_build(ORE, CLAY, bots, material, orecost, prevcost)) {
			build_bot(ORE, material, orecost, prevcost);
			++build[ORE];
		}
		*/

		// collect
		for (int ii = 0; ii < 4; ++ii) {
			if (bots[ii]) {
				material[ii] += bots[ii];
				printf("%d %s-bots collect %d %s; you now have %d %s\n",
						bots[ii], names[ii], bots[ii], names[ii], material[ii], names[ii]);
			}
		}
		// finish build
		for (int ii = 0; ii < 4; ++ii) {
			if (build[ii]) {
				bots[ii] += build[ii];
				printf("Succesfully built %d %s-bots."
						" You now have %d of them\n",
						build[ii], names[ii], bots[ii]);
			}
		}

		printf("Ending materials: %d ore, %d clay, %d obs, %d geo\n", material[0], material[1], material[2], material[3]);
		printf("Ending bots:      %d ore, %d clay, %d obs, %d geo\n", bots[0], bots[1], bots[2], bots[3]);
	}
	return material[3];
}

int main(int argc, char* argv[]) {
	int sum = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int id = read_int(&l);
		exec_blueprint(l);
	}
	free(line);

	//printf("%d\n", count);
	return 0;
}
