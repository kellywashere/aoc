#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

enum content {
	EMPTY,
	FLOOR,
	WALL
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

struct cell {
	enum content content;
};

struct grid {
	int          gridw;
	int          gridh;
	struct cell* grid;

	int          startidx;
	int          targetidx;

	int          nr_portals; // including AA and ZZ
	int*         portalidx; // [0]: idx of AA, [1]: idx of ZZ, [2]: first outside portal, [3] the inside portal of [2], etc
};

#define MAX_LINES 256
#define MAX_LINE_LEN 256

#define LABEL_AA 1000
#define LABEL_ZZ (1000 + 26*26-1)

/************************* MINHEAP *******************************/
// NOTE: this implementation allows searching for value, and
// changing key of a value to allow A-star search !
struct minheap_el {
	int key;
	int val;
};

struct minheap {
	struct minheap_el* mem;      // array-based min heap
	unsigned int       size;     // nr elements currently in heap
	unsigned int       capacity; // capacity in nr of elements
};

struct minheap* create_minheap() {
	struct minheap* heap = malloc(sizeof(struct minheap));
	heap->capacity = 127; // 7 layers start size
	heap->mem = malloc(sizeof(struct minheap_el) * heap->capacity);
	heap->size = 0;
	return heap;
}

void destroy_minheap(struct minheap* heap) {
	if (heap)
		free(heap->mem);
	free(heap);
}

static void minheap_swap(struct minheap* heap, int idx1, int idx2) {
	struct minheap_el t = heap->mem[idx1];
	heap->mem[idx1] = heap->mem[idx2];
	heap->mem[idx2] = t;
}

static void minheap_bubble_up(struct minheap* heap, int idx) {
	int idx_parent = (idx - 1) / 2;
	while (idx != 0 && heap->mem[idx].key < heap->mem[idx_parent].key) {
		minheap_swap(heap, idx, idx_parent);
		idx = idx_parent;
		idx_parent = (idx - 1) / 2;
	}
}

static void minheap_bubble_down(struct minheap* heap, int idx) {
	int key_this = heap->mem[idx].key;
	bool done = false;
	while (!done) {
		done = true;
		int smallest_key = key_this;
		int idx_smallest = idx;
		// find smallest between idx, left and right child
		int idx_child;
		for (idx_child = (idx * 2) + 1; idx_child <= (idx * 2) + 2; ++idx_child) {
			if (idx_child < heap->size) {
				int k = heap->mem[idx_child].key;
				if (k < smallest_key) {
					smallest_key = k;
					idx_smallest = idx_child;
				}
			}
		}
		if (idx_smallest != idx) {
			done = false;
			minheap_swap(heap, idx, idx_smallest);
			idx = idx_smallest;
		}
	}
}

void minheap_insert(struct minheap* heap, int key, int val) {
	// check capacity
	if (heap->size == heap->capacity) {
		heap->capacity = 2 * heap->capacity + 1; // 1 more layer
		heap->mem = realloc(heap->mem, sizeof(struct minheap_el) * heap->capacity);
	}
	int idx = heap->size; // insertion idx
	// copy info into new last element
	heap->mem[idx].key = key;
	heap->mem[idx].val = val;
	++heap->size;
	minheap_bubble_up(heap, idx);
}

int minheap_extract(struct minheap* heap, int* val) { // returns key, copies val to *val
	if (heap->size == 0) {
		fprintf(stderr, "Error: extracting from empty heap\n");
		exit(1);
	}
	int retKey = heap->mem[0].key;
	if (val)
		*val = heap->mem[0].val;

	--heap->size;
	if (heap->size > 0) {
		// copy ex-last element to root position
		heap->mem[0] = heap->mem[heap->size];
		minheap_bubble_down(heap, 0);
	}
	return retKey;
}

bool minheap_contains_val(struct minheap* heap, int val) {
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val)
			return true;
	}
	return false;
}

bool minheap_update_key_of_val(struct minheap* heap, int key, int val) {
	// Note: only changes key of first val found that had a different key
	// returns whether change was made
	bool changed = false;
	for (int idx = 0; idx < heap->size; ++idx) {
		if (heap->mem[idx].val == val && heap->mem[idx].key != key) {
			int oldkey = heap->mem[idx].key;
			heap->mem[idx].key = key;
			changed = true;
			if (key < oldkey)
				minheap_bubble_up(heap, idx);
			else
				minheap_bubble_down(heap, idx);
		}
	}
	return changed;
}

/************************* END MINHEAP ***************************/

// Use Dijkstra since we have no good heuristic here (A-star + h = 0 --> Dijkstra)

int dir2dx[]         = {1, 0, -1, 0};
int dir2dy[]         = {0, -1, 0, 1};

// Use Dijkstra since we have no good heuristic here (A-star + h = 0 --> Dijkstra)
int hscore(struct grid* g, int x, int y) {
	return 0;
}

int astar(struct grid* g, int fromidx, int toidx) {
	// Implemented as A-star search with h() == 0
	// nodes are (x,y), encoded in integer:
	// y * w + x  (0<=y<h, 0<=x<w)
	int w = g->gridw;
	int h = g->gridh;

	struct minheap* open_set = create_minheap();
	int* gscore = malloc(w * h * sizeof(int));
	int fscore;
	for (int ii = 0; ii < w * h; ++ii) {
		gscore[ii] = INT_MAX;
	}

	int idx = fromidx;
	gscore[idx] = 0;
	//fscore[idx] = hscore(g, g->startpos.x, g->startpos.y)
	fscore = hscore(g, idx % w, idx / w);

	minheap_insert(open_set, fscore, idx);

	int cost = INT_MAX;
	while (cost == INT_MAX && open_set->size) {
		minheap_extract(open_set, &idx);
		int y = idx / w;
		int x = idx % w;
		if (idx == toidx)
			cost = gscore[idx]; // we're done here!
		else {
			// for each neighbor of current
			for (int dir = 0; dir < 4; ++dir) { // 4 neighboring cells
				int nx = x + dir2dx[dir];
				int ny = y + dir2dy[dir];
				if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
					int nidx = ny * w + nx;
					// portal? Note: additional step!
					// check if we can travel to neighbor
					if (g->grid[nidx].content == FLOOR) {
						int tentative_gscore = gscore[idx] + 1;
						if (tentative_gscore < gscore[nidx]) {
							gscore[nidx] = tentative_gscore;
							fscore = tentative_gscore + hscore(g, nx, ny);
							if (!minheap_contains_val(open_set, nidx))
								minheap_insert(open_set, fscore, nidx);
							else
								minheap_update_key_of_val(open_set, fscore, nidx); // Nasty...
						}
					}
				}
			}
		}
	}

	destroy_minheap(open_set);
	return cost;
}

void delete_grid(struct grid* g) {
	if (g) {
		free(g->grid);
		free(g->portalidx);
		free(g);
	}
}

bool is_outside_portal(struct grid* g, int idx) {
	// returns true if idx resides on outside of grid
	int x = idx % g->gridw;
	int y = idx / g->gridw;
	return (x == 0 || y == 0 || x == g->gridw - 1 || y == g->gridh - 1);
}

struct grid* lines2grid(char lines[][MAX_LINE_LEN], int nr_lines) {
	// Find maze borders, not including labels
	int firstline = 0;
	while (!strchr(lines[firstline], '#'))
		++firstline;
	int lastline = firstline;
	while (strchr(lines[lastline], '#'))
		++lastline;
	--lastline;
	int firstchar = INT_MAX;
	int lastchar = 0;
	for (int l = firstline; l <= lastline; ++l) {
		int first = strchr(lines[l], '#') - lines[l];
		int last = strrchr(lines[l], '#') - lines[l];
		firstchar = first < firstchar ? first : firstchar;
		lastchar = last > lastchar ? last : lastchar;
	}
	// create maze
	struct grid* g = malloc(sizeof(struct grid));
	g->gridw = lastchar - firstchar + 1;
	g->gridh = lastline - firstline + 1;
	g->grid = malloc(g->gridw * g->gridh * sizeof(struct cell));
	// copy maze
	int nr_labels = 0;
	int* labels = malloc(g->gridw * g->gridh * sizeof(int));
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int linenr = firstline + y;
			int charnr = firstchar + x;
			char c = lines[linenr][charnr];
			int idx = y * g->gridw + x;
			g->grid[idx].content = EMPTY;
			labels[idx] = 0;
			if (c == '#')
				g->grid[idx].content = WALL;
			else if (c == '.') {
				g->grid[idx].content = FLOOR; // TODO: merge EMPTY and FLOOR?
				// Label attached? Labels get high numbers, to simplify relabeling algo later
				if (linenr >= 2 && isalpha(lines[linenr - 1][charnr]))
					labels[idx] = LABEL_AA + 26 * (lines[linenr - 2][charnr] - 'A') + (lines[linenr - 1][charnr] - 'A');
				else if (linenr + 2 < nr_lines && isalpha(lines[linenr + 1][charnr]))
					labels[idx] = LABEL_AA + 26 * (lines[linenr + 1][charnr] - 'A') + (lines[linenr + 2][charnr] - 'A');
				else if (charnr >= 2 && isalpha(lines[linenr][charnr - 1]))
					labels[idx] = LABEL_AA + 26 * (lines[linenr][charnr - 2] - 'A') + (lines[linenr][charnr - 1] - 'A');
				else if (charnr + 2 < MAX_LINE_LEN && isalpha(lines[linenr][charnr + 1]))
					labels[idx] = LABEL_AA + 26 * (lines[linenr][charnr + 1] - 'A') + (lines[linenr][charnr + 2] - 'A');
				nr_labels += labels[idx] ? 1 : 0; // count labels
			}
		}
	}
	//int*         portalidx; // [0]: idx of AA, [1]: idx of ZZ, [2]: first outside portal, [3] the inside portal of [2], etc
	g->nr_portals = nr_labels;
	g->portalidx = malloc(nr_labels * sizeof(int));

	int labelidx = 0;
	for (int ii = 0; ii < g->gridw * g->gridh; ++ii) {
		if (labels[ii] == LABEL_AA) {
			g->startidx = ii;
			g->portalidx[0] = ii;
			labels[ii] = 0;
		}
		else if (labels[ii] == LABEL_ZZ) {
			g->targetidx = ii;
			g->portalidx[1] = ii;
			labels[ii] = 0;
		}
		else if (labels[ii]) { // find other end of portal
			++labelidx;
			for (int jj = ii + 1; jj < g->gridw * g->gridh; ++jj) {
				if (labels[jj] == labels[ii]) {
					// first time: labelidx = 1 --> idx in portalidx = 2 * labelidx (out), and 2 * labelidx + 1 (in)
					g->portalidx[2 * labelidx + (is_outside_portal(g, ii) ? 0 : 1)] = ii;
					g->portalidx[2 * labelidx + (is_outside_portal(g, jj) ? 0 : 1)] = jj;
					labels[ii] = 0;
					labels[jj] = 0;
					break;
				}
			}
		}
	}

	return g;
}

void print_grid(struct grid* g) {
	// print
	for (int y = 0; y < g->gridh; ++y) {
		for (int x = 0; x < g->gridw; ++x) {
			int idx = y * g->gridw + x;
			struct cell* c = &g->grid[idx];
			if (c->content == WALL)
				printf("#");
			else if (c->content == EMPTY)
				printf(" ");
			else { // FLOOR
				bool isportal = false;
				int portal = 0;
				for (int ii = 0; ii < g->nr_portals; ++ii) {
					if (idx == g->portalidx[ii]) {
						portal = ii;
						isportal = true;
					}
				}
				if (isportal) {
					if (portal == 0)
						printf("A");
					else if (portal == 1)
						printf("Z");
					else
						printf("%c", 'a' + portal/2 - 1);
				}
				else
					printf(".");
			}
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	// read all line and remember them
	char lines[MAX_LINES][MAX_LINE_LEN];
	int nr_lines = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1)
		if (!empty_line(line))
			strcpy(lines[nr_lines++], line);
	free(line);

	struct grid* g = lines2grid(lines, nr_lines);
	print_grid(g);

	// algo: we assume some maximum level, say lmax
	// at that level, we enter the maze through an outside portal, and exit it through another outside portal
	// we can abstract lmax by just all possible distances from portal to portal
	// one level lower, we enter through some outside portal, we enter lmax once or multiple times, and exit through another outside portal
	// we can again abstract lmax-1 by just all possible distances from portal to portal
	// working our way down, we eventually have a connection from AA to ZZ

	// first, build a distance map from all portals to all portals
	// instance refers to: one instance of the maze (non-recursive)
	int* instancedistmap = calloc(g->nr_portals * g->nr_portals, sizeof(int));
	for (int ii = 0; ii < g->nr_portals - 1; ++ii) {
		for (int jj = ii + 1; jj < g->nr_portals; ++jj) {
			int from = g->portalidx[ii];
			int to = g->portalidx[jj];
			int cost = astar(g, from, to);
			instancedistmap[ii * g->nr_portals + jj] = cost;
			instancedistmap[jj * g->nr_portals + ii] = cost;
			char fromc = ii == 0 ? 'A' : ii == 1 ? 'Z' : 'a' - 1 + ii/2;
			char toc = jj == 0 ? 'A' : jj == 1 ? 'Z' : 'a' - 1 + jj/2;
			printf("From %c to %c: %d\n", fromc, toc, cost);
		}
	}
	int nr_portals = g->nr_portals;

	// outside port indices: 0, 1, 2, 4, 6, 8, ..., nr_portals - 2 (even nrs, and 1)
	// inside port indices: 3, 5, 7, ..., nr_portals - 1 (odd nrs except 1)

	int* prevdistmap = malloc(g->nr_portals * g->nr_portals * sizeof(int));
	int* curdistmap = malloc(g->nr_portals * g->nr_portals * sizeof(int));
	// we only care about the outside portals for the prev and cur versions
	memcpy(curdistmap, instancedistmap, g->nr_portals * g->nr_portals * sizeof(int));

	while (curdistmap[0 * g->nr_portals + 1] == INT_MAX) { // we cannot reach ZZ from AA
		// add one level of recursion AROUND current maze
		// prevdistmap <-- curdistmap
		int* t = prevdistmap;
		prevdistmap = curdistmap;
		curdistmap = prevdistmap; // swap, so we can reuse map without copying every time

		// for any inside-portal in curdistmap to any other inside-portal, we can choose if we go through prev or not
		for (int ii = 3; ii < g->nr_portals; ii += 2) { // inside portal of instancedistmap
			for (int jj = ii + 2; jj < nr_portals; jj += 2) { // another inside portal of instancedistmap
				int dist1 = instancedistmap[ii * nr_portals + jj]; // travel through instmap
				int dist2 = prevdistmap[(ii - 1) * nr_portals + (jj - 1)]; // travel through prev
				curdistmap[ii * nr_portals + jj] = dist1 < dist2 ? dist1 : dist2;
			}
		}
		
		// Now find shortest paths from any outside node of cur to any other outside node of cur
		// run Dijkstra for each output node to each other output node
		
		// outside port indices: 0, 1, 2, 4, 6, 8, ..., nr_portals - 2 (even nrs, and 1)
		int source = 0;
		while (source <= nr_portals - 4) {
			int dest = source < 2 ? source + 1 : source + 2;
			while (dest <= nr_portals - 2) {
				// run dijkstra
				dest += dest < 2 ? 1 : 2;
			}
			source += source < 2 ? 1 : 2;
		}

		 1  function Dijkstra(Graph, source):
 2      
 3      for each vertex v in Graph.Vertices:
 4          dist[v] ← INFINITY
 5          prev[v] ← UNDEFINED
 6          add v to Q
 7      dist[source] ← 0
 8      
 9      while Q is not empty:
10          u ← vertex in Q with min dist[u]
11          remove u from Q
12          
13          for each neighbor v of u still in Q:
14              alt ← dist[u] + Graph.Edges(u, v)
15              if alt < dist[v]:
16                  dist[v] ← alt
17                  prev[v] ← u
18
19      return dist[], prev[]
	



		// new distances, XX(out) --> YY(out) directly, or XX(out) --> QQ(in)=>QQ(out) --> RR(out)=>RR(in) --> YY(out)


	}

	// find distances from AA to all portals
	int cost = instancedistmap[1];
	printf("%d\n", cost);

	delete_grid(g);
	free(instancedistmap);
	free(curdistmap);
	free(prevdistmap);

	return 0;
}
