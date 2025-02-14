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

struct state {
	int bots[4];
	int material[4];

	// building bots this turn
	int built[4];
};

struct prices {
	int orecost[4];
	int prevcost[4];
};


bool can_build(enum mat b, struct state* s, struct prices* p) {
	return (s->material[0] >= p->orecost[b] &&
			(p->prevcost[b] == 0 || s->material[b - 1] >= p->prevcost[b]) );
}

void build_bot(enum mat b, struct state* s, struct prices* p, bool verbose) {
	if (verbose) {
		printf("Spend %d ore", p->orecost[b]);
		if (p->prevcost[b])
			printf(" and %d %s", p->prevcost[b], names[b - 1]);
		printf(" to start building a %s-bot.\n", names[b]);
	}
	s->material[0] -= p->orecost[b];
	if (p->prevcost[b])
		s->material[b - 1] -= p->prevcost[b];
	++s->built[b]; // bot not available yet!!!
}

void undo_build_bot(enum mat b, struct state* s, struct prices* p, bool verbose) {
	if (verbose) {
		printf("Undoing building a %s-bot. Getting back %d ore", names[b], p->orecost[b]);
		if (p->prevcost[b])
			printf(" and %d %s", p->prevcost[b], names[b - 1]);
		printf(".\n");
	}
	s->material[0] += p->orecost[b];
	if (p->prevcost[b])
		s->material[b - 1] += p->prevcost[b];
	--s->built[b];
}

/*
int calc_turns_to_build(enum mat b, int bots[], int material[], int orecost[], int prevcost[]) {
	int needed = orecost[b] - material[ORE];
	int turns_ore = needed <= 0 ? 0 : CEILDIV(needed, bots[ORE]);
	needed = prevcost[b] ? prevcost[b] - material[b - 1] : 0;
	int turns_other = needed <= 0 ? 0 :
		(bots[b - 1] == 0 ? INT_MAX : CEILDIV(needed, bots[b - 1]));
	return MAX(turns_ore, turns_other);
}
*/

/*
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
		return turns_build < turns_nobuild;
	}
	return false;
}
*/

#define MAX_TURNS 24

int max_geode(int turn, int minbot, struct state* state, struct prices* prices) {
	// minbot is var needed to pass state info about build decisions
	// 0: turn is just starting, 1: we cannot build bot ORE anymore, etc
	if (turn > MAX_TURNS)
		return state->material[GEODE];

	printf("%*sState:\n", turn, "");
	printf("%*sBots: %d %d %d %d\n", turn, "", state->bots[0], state->bots[1], state->bots[2], state->bots[3]);
	printf("%*sMaterial: %d %d %d %d\n", turn, "", state->material[0], state->material[1], state->material[2], state->material[3]);

	// BUILD
	if (can_build(GEODE, state, prices))
		build_bot(GEODE, state, prices, false);

	// figure out all options recursively... :(
	for (int b = minbot; b <= OBSIDIAN; ++b) {
		if (can_build(b, state, prices)) { // two options
			// do not build:
			int geode_nobuild = max_geode(turn, b + 1, state, prices);

			// or build:
			build_bot(b, state, prices, false);
			int geode_build = max_geode(turn, b + 1, state, prices);
			undo_build_bot(b, state, prices, false);
			return MAX(geode_build, geode_nobuild);
		}
	}
	
	struct state nextstate;
	for (int ii = ORE; ii <= GEODE; ++ii) {
		nextstate.bots[ii] = state->bots[ii] + state->built[ii];
		nextstate.material[ii] = state->material[ii] + state->bots[ii];
		nextstate.built[ii] = 0;
	}
	return max_geode(turn + 1, 0, &nextstate, prices);
}

int playout(int orebots, int claybots, int obsbots, struct prices* prices) {
	struct state state;
	state.bots[0] = 1;
	for (int ii = 1; ii < 4; ++ii)
		state.bots[ii] = 0;
	for (int ii = 0; ii < 4; ++ii) {
		state.material[ii] = 0;
		state.built[ii] = 0;
	}

	for (int t = 0; t < MAX_TURNS; ++t) {
		// build?
		if (can_build(GEODE, &state, prices))
			build_bot(GEODE, &state, prices, true);
		if (state.bots[OBSIDIAN] < obsbots && can_build(OBSIDIAN, &state, prices))
			build_bot(OBSIDIAN, &state, prices, true);
		if (state.bots[CLAY] < claybots && can_build(CLAY, &state, prices))
			build_bot(CLAY, &state, prices, true);
		if (state.bots[ORE] < orebots && can_build(ORE, &state, prices))
			build_bot(ORE, &state, prices, true);
		for (int ii = ORE; ii <= GEODE; ++ii) {
			state.material[ii] = state.material[ii] + state.bots[ii];
			state.bots[ii] = state.bots[ii] + state.built[ii];
			state.built[ii] = 0;
		}
		/*
		printf("Turn: %d\n", t);
		printf("Bots: %d %d %d %d\n", state.bots[0], state.bots[1], state.bots[2], state.bots[3]);
		printf("Material: %d %d %d %d\n", state.material[0], state.material[1], state.material[2], state.material[3]);
		*/

	}

	return state.material[GEODE];
}

int exec_blueprint(char* line) {
	// strat: build geode-bot whenever possible
	// build obs-bot when it results in new geode-bot faster
	// build clay bot when it results in obs-bot faster
	// build ore-bot when it results in clay bot faster (?)
	// "faster" is evaluated in simplistic way here
	struct prices prices;
	prices.orecost[0] = read_int(&line);
	prices.prevcost[0] = 0;
	prices.orecost[1] = read_int(&line);
	prices.prevcost[1] = 0;
	prices.orecost[2] = read_int(&line);
	prices.prevcost[2] = read_int(&line);
	prices.orecost[3] = read_int(&line);
	prices.prevcost[3] = read_int(&line);

	printf("Blueprint:\n");
	printf("  Each ore robot costs %d ore.\n", prices.orecost[0]);
	printf("  Each clay robot costs %d ore.\n", prices.orecost[1]);
	printf("  Each obsidian robot costs %d ore and %d clay.\n", prices.orecost[2], prices.prevcost[2]);
	printf("  Each geode robot costs %d ore and %d obsidian.\n", prices.orecost[3], prices.prevcost[3]);

	for (int orebots = 1; orebots < 4; ++orebots) {
		for (int claybots = 1; claybots < 4; ++claybots) {
			for (int obsbots = 1; obsbots < 4; ++obsbots) {
				int geode = playout(orebots, claybots, obsbots, &prices);
				printf("Geode: %d\n", geode);
			}
		}
	}
	return 0;
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
