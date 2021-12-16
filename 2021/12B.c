#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_VERTICES          32
#define MAX_EDGES_PER_VERTEX  16

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

// Note: for simplicity edges are part of vertex itself
struct vertex {
	char* name;
	bool  big;
	int   edges[MAX_EDGES_PER_VERTEX];
	int   nr_edges;
	int   times_visited;
};

struct graph {
	struct vertex vertices[MAX_VERTICES];
	int    nr_vertices;
};

int vertexname_to_idx(char* name, struct graph* g) {
	// if name does not exists, creates new vertex
	int ii;
	for (ii = 0; ii < g->nr_vertices; ++ii)
		if (!strcmp(g->vertices[ii].name, name))
			return ii;
	// create vertex
	if (ii >= MAX_VERTICES) {
		fprintf(stderr, "Too many vertices\n");
		return -1;
	}
	ii = g->nr_vertices++;
	int l = strlen(name);
	g->vertices[ii].name = malloc((l + 1) * sizeof(char));
	strcpy(g->vertices[ii].name, name);
	g->vertices[ii].nr_edges = 0;
	g->vertices[ii].big = name[0] >= 'A' && name[0] <= 'Z';
	// terminate list
	g->vertices[ii + 1].name = NULL;
	return ii;
}

void process_line(char* line, struct graph* g) {
	char* t = strtok(line, "-");
	int idx1 = vertexname_to_idx(t, g);
	t = strtok(NULL, " \t\n");
	int idx2 = vertexname_to_idx(t, g);
	// add edges
	g->vertices[idx1].edges[g->vertices[idx1].nr_edges++] = idx2;
	g->vertices[idx2].edges[g->vertices[idx2].nr_edges++] = idx1;
}

// debug
void print_graph(struct graph* g) {
	printf("Graph:\n");
	int ii, jj;
	for (ii = 0; ii < g->nr_vertices; ++ii) {
		printf("%s:\n", g->vertices[ii].name);
		for (jj = 0; jj < g->vertices[ii].nr_edges; ++jj) {
			printf(" --> %s\n", g->vertices[g->vertices[ii].edges[jj]].name);
		}
	}
}

int count_paths(struct graph* g, int cur, int target, bool small_visited_twice) {
	if (cur == target)
		return 1;
	int count = 0;
	int edge_idx;
	struct vertex* v = &g->vertices[cur];
	++v->times_visited;
	if (!v->big && v->times_visited == 2)
		small_visited_twice = true;
	for (edge_idx = 0; edge_idx < v->nr_edges; ++edge_idx) {
		int n = v->edges[edge_idx];
		struct vertex* nv = &g->vertices[n];
		if (nv->big || nv->times_visited < 1 ||
				(nv->times_visited < 2 && !small_visited_twice) ) {
			count += count_paths(g, n, target, small_visited_twice);
		}
	}
	if (!v->big && v->times_visited == 2)
		small_visited_twice = false;
	--v->times_visited;
	return count;
}

int main(int argc, char* argv[]) {
	struct graph graph;
	graph.nr_vertices = 0;
	int ii;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		process_line(line, &graph);
	}
	int start_idx = vertexname_to_idx("start", &graph);
	graph.vertices[start_idx].times_visited = 666; // dirty hack to prevent start being visited twice
	int end_idx = vertexname_to_idx("end", &graph);
	int count = count_paths(&graph, start_idx, end_idx, false);
	free(line);
	printf("%d\n", count);

	// clean up
	for (ii = 0; graph.vertices[ii].name != NULL; ++ii)
		free(graph.vertices[ii].name);
	return 0;
}
