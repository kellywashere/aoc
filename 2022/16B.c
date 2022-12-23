#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define MAX_VERTICES   64
#define MAX_NEIGHBORS  64

#define MAX_NAMES MAX_VERTICES

#define MAX_PLAYERS 2

struct edge {
	int to; // idx of vertex this edge ends at
	int len;
};

struct vertex {
	char        name[4];
	int         nr_edges;
	struct edge edge[MAX_NEIGHBORS];
	int         flow;
	bool        open;

	// Dijkstra:
	int         dist;
	int         prev;
	bool        visited;
};

struct graph {
	int           nr_vertices;
	struct vertex vertex[MAX_VERTICES];
};

int name_to_idx(char* name, char* names[]) {
	// if name not in names, adds it
	int ii;
	for (ii = 0; ii < MAX_NAMES && names[ii] != NULL; ++ii)
		if (!strcmp(names[ii], name))
			return ii;
	if (ii < MAX_NAMES) {
		int l = strlen(name);
		names[ii] = malloc((l + 1) * sizeof(char));
		strcpy(names[ii], name);
		return ii;
	}
	fprintf(stderr, "Ran out of space to store name %s!\n", name);
	return -1;
}

int read_int(char* line) {
	while (*line && !isdigit(*line))
		++line;
	int x = 0;
	while (isdigit(*line)) {
		x = 10 * x + *line - '0';
		++line;
	}
	return x;
}

bool read_vertex_name(char** pLine, char* name) {
	while (**pLine && !(isupper(**pLine) && isupper(*(*pLine + 1))))
		++(*pLine);
	if (!**pLine)
		return false;
	strncpy(name, *pLine, 2);
	name[2] = '\0';
	*pLine += 2;
	return true;
}

struct graph* read_graph(FILE* fp) {
	char* nodes[MAX_VERTICES + 1]; // NULL terminated char* array
	for (int ii = 0; ii < MAX_NAMES + 1; ++ii)
		nodes[ii] = NULL; // put all terminations in place

	struct graph* g = malloc(sizeof(struct graph));
	g->nr_vertices = 0;

	char name[4];
	// collect info from input
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) != -1) {
		char* l = line;
		read_vertex_name(&l, name);
		int vertex_idx = name_to_idx(name, nodes);
		g->nr_vertices = vertex_idx + 1 > g->nr_vertices ? vertex_idx + 1 : g->nr_vertices;

		struct vertex* v = &g->vertex[vertex_idx];

		strncpy(v->name, name, 4);
		v->flow = read_int(line);
		v->open = false;

		v->nr_edges = 0;
		while (read_vertex_name(&l, name)) {
			v->edge[v->nr_edges].to = name_to_idx(name, nodes);
			v->edge[v->nr_edges].len = 1;
			++v->nr_edges;
		}
	}
	for (int ii = 0; ii < MAX_VERTICES && nodes[ii] != NULL; ++ii)
		free(nodes[ii]);
	free(line);
	return g;
}

void show_graph(struct graph* g) {
	for (int ii = 0; ii < g->nr_vertices; ++ii) {
		printf("%s: ", g->vertex[ii].name);
		struct vertex* v = &g->vertex[ii];
		for (int jj = 0; jj < v->nr_edges; ++jj) {
			printf("%s(%d),", g->vertex[v->edge[jj].to].name, v->edge[jj].len);
		}
		printf("\n");
	}
}

void dijkstra(struct graph* g, int s) {
	// printf("Start node: %d\n", s);
	// s is idx of start node
	for (int ii = 0; ii < g->nr_vertices; ++ii) {
		g->vertex[ii].dist = INT_MAX;
		g->vertex[ii].prev = -1;
		g->vertex[ii].visited = false;
	}
	g->vertex[s].dist = 0;
	int nr_unvisited = g->nr_vertices;
	while (nr_unvisited) {
		int cur = 0;
		int min_dist = INT_MAX;
		for (int ii = 0; ii < g->nr_vertices; ++ii) {
			if (!g->vertex[ii].visited && g->vertex[ii].dist < min_dist) {
				cur = ii;
				min_dist = g->vertex[ii].dist;
			}
		}
		// printf("Taking node %d\n", cur);
		struct vertex* v = &g->vertex[cur];
		v->visited = true;
		--nr_unvisited;
		for (int ii = 0; ii < v->nr_edges; ++ii) {
			int to = v->edge[ii].to;
			int len = v->edge[ii].len;
			if (!g->vertex[to].visited) {
				int alt = v->dist + len;
				if (alt < g->vertex[to].dist) {
					g->vertex[to].dist = alt;
					g->vertex[to].prev = cur;
				}
			}
		}
	}
}

struct graph* simplify_graph(struct graph* g_in, int startidx) {
	struct graph* g_out = malloc(sizeof(struct graph));
	// step 0: copy over start node
	struct vertex* v_in = &g_in->vertex[startidx];
	struct vertex* v_out = &g_out->vertex[0];
	strcpy(v_out->name, v_in->name);
	v_out->flow = v_in->flow;
	v_out->nr_edges = 0;
	g_out->nr_vertices = 1;
	// step 1: copy over all flow nodes into g_out, without edges
	for (int ii = 0; ii < g_in->nr_vertices; ++ii) {
		if (g_in->vertex[ii].flow && ii != startidx) {
			v_in = &g_in->vertex[ii];
			v_out = &g_out->vertex[g_out->nr_vertices];
			strcpy(v_out->name, v_in->name);
			v_out->flow = v_in->flow;
			v_out->open = v_in->open;
			v_out->nr_edges = 0;
			++g_out->nr_vertices;
		}
	}
	// step 2: add all edges
	for (int ii = 0; ii < g_out->nr_vertices; ++ii) {
		v_out = &g_out->vertex[ii];
		// find corresponding node in g_in
		int in_idx = 0;
		for (; strcmp(g_out->vertex[ii].name, g_in->vertex[in_idx].name) != 0; ++in_idx);
		// find all distances
		dijkstra(g_in, in_idx);
		// create edges to all other flow nodes
		for (int jj = 0; jj < g_in->nr_vertices; ++jj) {
			if (jj != in_idx && g_in->vertex[jj].flow) {
				// find corresponding node in g_out
				int out_idx = 0;
				for (; strcmp(g_in->vertex[jj].name, g_out->vertex[out_idx].name) != 0; ++out_idx);
				// create edge in g_out with correct len
				v_out->edge[v_out->nr_edges].to = out_idx;
				v_out->edge[v_out->nr_edges].len = g_in->vertex[jj].dist;
				++v_out->nr_edges;
			}
		}
	}
	return g_out;
}

int find_max_reward(struct graph* g, int nr_players, int posidx[], int turns_left[]) {
	int player = 0;
	while (turns_left[player] == 0 && player < nr_players)
		++player;
	if (player == nr_players)
		return 0;
	struct vertex* vcur = &g->vertex[posidx[player]];
	// iterate over all edges leading to a closed flow node
	int max_reward = 0;
	// option 1: player stops, and lets next player continue
	if (player + 1 < nr_players) { // next player
		int turnsleft_bu = turns_left[player]; // for undo
		turns_left[player] = 0;
		max_reward = find_max_reward(g, nr_players, posidx, turns_left);
		turns_left[player] = turnsleft_bu;
	}
	// option 2: this player makes at least one more move
	for (int e_idx = 0; e_idx < vcur->nr_edges; ++e_idx) {
		struct edge* e = &vcur->edge[e_idx];
		struct vertex* vto = &g->vertex[e->to];
		if (vto->flow && !vto->open && e->len + 1 < turns_left[player]) { // otherwise no point in going there
			// follow edge, then recurse, then undo
			vto->open = true;
			int pos_bu = posidx[player]; // for undo
			int turnsleft_bu = turns_left[player]; // for undo
			posidx[player] = e->to;
			turns_left[player] -= e->len + 1;
			int reward = turns_left[player] * vto->flow + find_max_reward(g, nr_players, posidx, turns_left);
			max_reward = reward > max_reward ? reward : max_reward;
			// undo
			posidx[player] = pos_bu;
			turns_left[player] = turnsleft_bu;
			vto->open = false;
		}
	}
	return max_reward;
}

int main(int argc, char* argv[]) {
	struct graph* g_in = read_graph(stdin);
	// find start node
	int start = 0;
	while (strcmp(g_in->vertex[start].name, "AA") != 0)
		++start;

	struct graph* g = simplify_graph(g_in, start);
	free(g_in);
	// show_graph(g);

	int curidx[MAX_PLAYERS] = {0}; // start node by design
	int players = 2;
	int turns_left[MAX_PLAYERS] = {0};
	for (int ii = 0; ii < players; ++ii)
		turns_left[ii] = 30 - 4 * (players - 1);
	int max_reward = find_max_reward(g, players, curidx, turns_left);
	printf("%d\n", max_reward);

	free(g);
	return 0;
}
