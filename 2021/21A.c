#include <stdio.h>

struct game {
	int pos[2];
	int score[2];
	int turn;
	int die;
	int nr_rolls;
};

int throw_die(struct game* g) {
	int d = g->die;
	g->die = (g->die % 100) + 1;
	++g->nr_rolls;
	return d;
}

int main(int argc, char* argv[]) {
	struct game game;
	scanf("Player %*d starting position: %d\n", &game.pos[0]);
	scanf("Player %*d starting position: %d\n", &game.pos[1]);
	game.score[0] = game.score[1] = 0;
	game.die = 1;
	game.turn = 0;
	game.nr_rolls = 0;

	while (game.score[0] < 1000 && game.score[1] < 1000) {
		int dtot = throw_die(&game) + throw_die(&game) + throw_die(&game);
		game.pos[game.turn] = (game.pos[game.turn] + dtot - 1) % 10 + 1;
		game.score[game.turn] += game.pos[game.turn];
		// printf("Player %d rolls a total of %d and lands on %d\n", game.turn + 1, dtot, game.pos[game.turn]);

		game.turn = 1 - game.turn;
	}
	int losing_score = game.score[0] < game.score[1] ? game.score[0] : game.score[1];
	printf("%d\n", losing_score * game.nr_rolls);


	return 0;
}
