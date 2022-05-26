#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// I first solved this one by hand!
// There is already one empty cell in starting grid
// Only possbile moves from start are 4 moves into that hole
// Strategy:
// 1. Move hole to left of cell containing goal data
// 2. Move goal data into hole
// 3. Use 4 moves to move hole down, left, left, up so it
// is again to the left of the data.
// [Note this only works if there is no "wall" in top row
// (which there isn't) otherwise we would have to move the hole
// underneath the goal data, and move goal data downward]
// So once the hole is to the left of the data, and there is no
// wall in top two rows, it takes another 5*(dist goal to target) + 1
// moves.
//
// Solution can be made much more robust
// e.g. if there is no initial hole: create one first, as close as possible to goal data
// For Astar I am not using minqueue, to make code simpler (but of course slower)

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && (*line < '0' || *line > '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

enum cell {
	HOLE,
	WALL,
	GOALDATA,
	DATA
};

struct drive {
	int used;
	int size;
	bool is_movable; // wall detection algo using flood fill
};

void show_drivegrid(struct drive* g, int w, int h) {
	for (int y = 0; y < h; ++y) {
		/*
		for (int x = 0; x < w; ++x)
			printf(" %d,%d", x%10,y%10);
		printf("\n");
		*/
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].used);
		printf("\n");
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].size - g[x + w * y].used); // avail
		printf("\n");
		for (int x = 0; x < w; ++x)
			printf("%4d", g[x + w * y].is_movable);
		printf("\n\n");
	}
}

void show_grid(enum cell* g, int w, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			enum cell c = g[x + y*w];
			printf("%c", c == HOLE ? 'x' :
			             c == WALL ? '#' :
			             c == GOALDATA ? 'G' :
			             '.');
		}
		printf("\n");
	}
}

/* ************ FLOODFILL for wall finding ********************* */
void floodfill_hole_rec(struct drive* g, int w, int h, int x, int y) {
	int idx = x + w * y;
	g[idx].is_movable = true;
	if (x > 0 && g[idx - 1].is_movable == 0 && g[idx - 1].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x - 1, y);
	if (x < w - 1 && g[idx + 1].is_movable == 0 && g[idx + 1].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x + 1, y);
	if (y > 0 && g[idx - w].is_movable == 0 && g[idx - w].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x, y - 1);
	if (y < h - 1 && g[idx + w].is_movable == 0 && g[idx + w].used <= g[idx].size)
		floodfill_hole_rec(g, w, h, x, y + 1);
}

// floodfill finds out where the hole can travel to (part of wall detection)
void floodfill_hole(struct drive* g, int w, int h) {
	// init for flood fill
	for (int ii = 0; ii < w * h; ++ii)
		g[ii].is_movable = g[ii].used == 0 ? true : false; // set hole(s) to true
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (g[x + w*y].used == 0)
				floodfill_hole_rec(g, w, h, x, y);
		}
	}
}
/* ************ end FLOODFILL for wall finding ********************* */

struct state { // state of nodes
	int holex;
	int holey;
	int goalx;
	int goaly;
};


int manhat_dist(int x1, int y1, int x2, int y2) {
	int dx = x2 > x1 ? x2 - x1 : x1 - x2;
	int dy = y2 > y1 ? y2 - y1 : y1 - y2;
	return dx + dy;
}

int hScore(struct state* s) {
	// make this monotonic to allow simpler Astar search, see:
	// https://en.wikipedia.org/wiki/Consistent_heuristic
	// that is why this fn is a bit complicated
	// it calculates *exactly* the nr of moves to bring goaldata to 0,0
	// assuming there are no walls in the way.
	// the easiest way to do this is to actually run the "simulation"
	int gx = s->goalx;
	int gy = s->goaly;
	int hx = s->holex;
	int hy = s->holey;
	int h = 0;
	// simulation loop
	while (gx > 0 || gy > 0) {
		// choose which is cheaper: move goaldata left or up?
		int costleft = manhat_dist(hx, hy, gx - 1, gy) + 1; // +1: swap goaldata and hole
		costleft += gy == hy && hx > gx ? 2 : 0; // move around goal data
		int costup = manhat_dist(hx, hy, gx, gy - 1) + 1; // +1: swap goaldata and hole
		costup += gx == hx && hy > gy ? 2 : 0; // move around goal data
		if (gy > 0 && costup < costleft) { // move up
			h += costup;
			hx = gx;
			hy = gy--;
		}
		else { // move left
			h += costleft;
			hx = gx--;
			hy = gy;
		}
	}
	return h;
}

struct Astarnode { // for openSet
	struct state      state;
	struct Astarnode* prev; // doubly linked list
	struct Astarnode* next;
};

struct Astarnode* dll_remove(struct Astarnode* head, struct Astarnode* n) {
	// Removes node n from chain, does not free!!
	if (n->next)
		n->next->prev = n->prev;
	if (n->prev)
		n->prev->next = n->next;
	else
		head = n->next;
	return head;
}

/***************** score hashmap ***********************/
// hash idx = holex + holy * gridw
// at each hash idx we have a linked list of scores

struct score { // gScore, fScore, hScore, for hash map storage
	// true state indicated by both hole and goal location
	struct state  state; // full state not really needed: hole loc already part of hash idx
	int           score;
	struct score* next; // linked list
};

struct score_hashmap {
	struct score** scores; // gridw * gridh sized array of linked lists
	int            gridw;
	int            gridh;
};

struct score_hashmap* create_score_hashmap(int w, int h) {
	struct score_hashmap* hm = malloc(sizeof(struct score_hashmap));
	hm->scores = calloc(w * h, sizeof(struct score*)); // init to NULL
	hm->gridw = w;
	hm->gridh = h;
	return hm;
}

void destroy_score_hashmap(struct score_hashmap* hm) {
	if (hm) {
		for (int idx = 0; idx < hm->gridw * hm->gridh; ++idx) {
			struct score* head = hm->scores[idx];
			while (head) {
				struct score* n = head;
				head = head->next;
				free(n);
			}
		}
		free(hm);
	}
}

void score_hashmap_add(struct score_hashmap* hm, struct state* state, int score) {
	int idx = state->holex + hm->gridw * state->holey;
	struct score* n = malloc(sizeof(struct score));
	n->state = *state;
	n->score = score;
	n->next = hm->scores[idx];
	hm->scores[idx] = n;
}

struct score* score_hashmap_find(struct score_hashmap* hm, struct state* state) {
	int idx = state->holex + hm->gridw * state->holey;
	for (struct score* n = hm->scores[idx]; n != NULL; n = n->next) {
		if (n->state.goalx == state->goalx && n->state.goaly == state->goaly)
			return n;
	}
	return NULL;
}
/***************** end score hashmap ***********************/



// move directions:
int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

int find_path(enum cell* g, int w, int h) {
	struct Astarnode* openSet   = NULL;
	struct score_hashmap* gScores = create_score_hashmap(w, h);
	struct score_hashmap* fScores = create_score_hashmap(w, h);
	// add starting node to openset
	openSet = malloc(sizeof(struct Astarnode));
	openSet->prev = NULL;
	openSet->next = NULL;
	// find hole and goal data
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int idx = x + y * w;
			if (g[idx] == HOLE) {
				openSet->state.holex = x;
				openSet->state.holey = y;
			}
			else if (g[idx] == GOALDATA) {
				openSet->state.goalx = x;
				openSet->state.goaly = y;
			}
		}
	}
	score_hashmap_add(gScores, &openSet->state, 0);
	score_hashmap_add(fScores, &openSet->state, hScore(&openSet->state));

	int found_gScore = -1;
	int debug_count = 0;
	while (found_gScore == -1 && openSet != NULL && debug_count < 10000) { // TODO: remove debugcount
		++debug_count;
		// find lowest fScore in openSet
		struct Astarnode* current = openSet;
		int lowest_fScore = INT_MAX;
		for (struct Astarnode* n = openSet; n != NULL; n = n->next) {
			struct score* fScore = score_hashmap_find(fScores, &n->state);
			if (fScore->score < lowest_fScore) {
				current = n;
				lowest_fScore = fScore->score;
			}
		}
		// printf("Current: gx,gy=%d,%d, hx,hy=%d,%d\n", current->state.goalx, current->state.goaly, current->state.holex, current->state.holey);
		struct score* gScore = score_hashmap_find(gScores, &current->state);
		// remove current from openSet
		openSet = dll_remove(openSet, current);
		// done?
		if (current->state.goalx == 0 && current->state.goaly == 0)
			found_gScore = gScore->score;
		else {
			// find neighbor nodes
			for (int ii = 0; ii < 4; ++ii) {
				struct state nbr = current->state;
				nbr.holex += dx[ii];
				nbr.holey += dy[ii];
				if (nbr.holex < 0 || nbr.holex >= w || nbr.holey < 0 || nbr.holey >= h || g[nbr.holex + nbr.holey * w] == WALL)
					continue;
				if (nbr.holex == nbr.goalx && nbr.holey == nbr.goaly) { // swap hole with goaldata
					nbr.goalx = current->state.holex;
					nbr.goaly = current->state.holey;
				}
				int tentative_gScore = gScore->score + 1;
				struct score* gScore_nbr = score_hashmap_find(gScores, &nbr);
				if (gScore_nbr == NULL || tentative_gScore < gScore_nbr->score) {
					// This path to neighbor is better than any previous one. Record it!
					if (gScore_nbr == NULL) {
						score_hashmap_add(gScores, &nbr, tentative_gScore);
						score_hashmap_add(fScores, &nbr, tentative_gScore + hScore(&nbr));
					}
					else { // we need to update the score instead. no need to recalc hScore
						int dg = gScore_nbr->score - tentative_gScore;
						gScore_nbr->score = tentative_gScore;
						struct score* fScore = score_hashmap_find(fScores, &nbr);
						fScore->score -= dg; // avoiud recalc of hScore
					}
					// try to find neighbor in openSet
					struct Astarnode* n = openSet;
					for ( ; n != NULL; n = n->next) {
						if (n->state.goalx == nbr.goalx && n->state.goaly == nbr.goaly && n->state.holex == nbr.holex && n->state.holey == nbr.holey)
							break;
					}
					if (n == NULL) { // create new node and add to openSet
						n = malloc(sizeof(struct Astarnode));
						n->state = nbr;
						n->prev = NULL; // attach as head
						n->next = openSet;
						if (openSet)
							openSet->prev = n;
						openSet = n;
						// printf("Adding to set: gx,gy=%d,%d, hx,hy=%d,%d\n", nbr.goalx, nbr.goaly, nbr.holex, nbr.holey);
					}
				}
			}
		}
		free(current);
	}
	// clean up
	while (openSet) {
		struct Astarnode* n = openSet;
		openSet = openSet->next;
		free(n);
	}
	destroy_score_hashmap(gScores);
	destroy_score_hashmap(fScores);

	return found_gScore;
}

// struct tmpnode only used for temp storage at data read in
struct tmpnode {
	int          x;
	int          y;
	int          size;
	int          used;
	struct tmpnode* next;
};

int main(int argc, char* argv[]) {
	struct tmpnode* tmpnodelist = NULL;
	int gridw = 0;
	int gridh = 0;

	// read stdin
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (line[0] == '/') {
			struct tmpnode* n = malloc(sizeof(struct tmpnode));
			char* l = line;
			n->x = read_int(&l);
			n->y = read_int(&l);
			n->size = read_int(&l);
			n->used = read_int(&l);
			n->next = tmpnodelist;
			tmpnodelist = n;
			gridw = (n->x+1) > gridw ? (n->x+1) : gridw;
			gridh = (n->y+1) > gridh ? (n->y+1) : gridh;
		}
	}
	free(line);

	// transform LL into temporary "drive grid"
	struct drive* drivegrid = calloc(gridw * gridh, sizeof(struct drive));
	for (struct tmpnode* n = tmpnodelist; n != NULL; n = n->next) {
		drivegrid[n->x + gridw * n->y].used = n->used;
		drivegrid[n->x + gridw * n->y].size = n->size;
	}
	while (tmpnodelist) {
		struct tmpnode* n = tmpnodelist;
		tmpnodelist = tmpnodelist->next;
		free(n);
	}

	floodfill_hole(drivegrid, gridw, gridh); // wall detection via floodfill

	// transform into puzzle drivegrid
	enum cell* grid = calloc(gridw * gridh, sizeof(enum cell));
	for (int ii = 0; ii < gridw * gridh; ++ii) {
		grid[ii] = ii == gridw - 1 ? GOALDATA :
		           drivegrid[ii].used == 0 ? HOLE :
		           !drivegrid[ii].is_movable ? WALL :
		           DATA;
	}
	free(drivegrid);

	// show_grid(grid, gridw, gridh);
	int moves = find_path(grid, gridw, gridh);
	printf("%d\n", moves);

	free(grid);
	return 0;
}
