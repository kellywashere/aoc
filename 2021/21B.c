#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#define NR_STATES (21 * 21 * 10 * 10)

int statenr_from_state(int pos[], int score[]) {
	return (score[1] * 21 + score[0]) * 100 + (pos[1] - 1) * 10 + (pos[0] - 1);
}

void state_from_statenr(int nr, int pos[], int score[]) {
	pos[0] = 1 + nr % 10;
	nr /= 10;
	pos[1] = 1 + nr % 10;
	nr /= 10;
	score[0] = nr % 21;
	score[1] = nr / 21;
}

int main(int argc, char* argv[]) {
	uint64_t state[2 * NR_STATES] = {0};

	int pos[2];
	int score[2] = {0, 0};

	scanf("Player %*d starting position: %d\n", &pos[0]);
	scanf("Player %*d starting position: %d\n", &pos[1]);

	int turn = 0;
	state[NR_STATES + statenr_from_state(pos, score)] = 1; // 1 universe, with this state
	
	int newpos[2];
	int newscore[2];
	int all_games_finished = false;
	uint64_t universes[2] = {0, 0};
	while (!all_games_finished) {
		all_games_finished = true;
		memset(state + turn * NR_STATES, 0, NR_STATES * sizeof(uint64_t)); // clear
		for (pos[1] = 1; pos[1] <= 10; ++pos[1]) {
			for (pos[0] = 1; pos[0] <= 10; ++pos[0]) {
				for (score[1] = 0; score[1] < 21; ++score[1]) { // < 21 !!
					for (score[0] = 0; score[0] < 21; ++score[0]) { // < 21 !!
						newpos[0] = pos[0];
						newpos[1] = pos[1];
						newscore[0] = score[0];
						newscore[1] = score[1];

						uint64_t n = state[(1 - turn) * NR_STATES + statenr_from_state(pos, score)];
						if (n)
							all_games_finished = false;
						int die[3];
						// TODO: single loop, + LUT for how many ways we can make e.g. sum of 5
						for (die[0] = 1; die[0] <= 3; ++die[0]) {
							for (die[1] = 1; die[1] <= 3; ++die[1]) {
								for (die[2] = 1; die[2] <= 3; ++die[2]) {
									int sum = die[0] + die[1] + die[2];
									newpos[turn] = (pos[turn] + sum - 1) % 10 + 1;
									newscore[turn] = (score[turn] + newpos[turn]);
									if (newscore[turn] >= 21)
										universes[turn] += n;
									else
										state[turn * NR_STATES + statenr_from_state(newpos, newscore)] += n;
								}
							}
						}
					}
				}
			}
		}

		turn = 1 - turn;
	}

	printf("Wins player 1: %" PRIu64 "\n", universes[0]);
	printf("Wins player 2: %" PRIu64 "\n", universes[1]);
	printf("%" PRIu64 "\n", universes[0] > universes[1] ? universes[0] : universes[1]);

	return 0;
}
