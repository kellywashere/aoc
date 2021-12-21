#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_SCANNERS 50
#define MAX_SCANCOORDS 50

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

struct coord {
	int x;
	int y;
	int z;
};

struct scanner {
	struct coord coords[MAX_SCANCOORDS];
	int          nr_coords;
	bool         abs_coords; // when true, coords are absolute
};

void rotate_coord(struct coord* c, int r) { // r = 0..23
	int x = c->x;
	int y = c->y;
	int z = c->z;
	int perm  = r / 4;
	int signs = r % 4;
	switch(perm) { // permuting axes, keep determinant of rot matrix same
		case 1:
			c->x = -c->x; c->y = z; c->z = y;
			break;
		case 2:
			c->x = y; c->y = x; c->z = -c->z;
			break;
		case 3:
			c->x = y; c->y = z; c->z = x;
			break;
		case 4:
			c->x = z; c->y = x; c->z = y;
			break;
		case 5:
			c->x = z; c->y = -c->y; c->z = x;
			break;
		default:
			break;
	}
	switch (signs) { // flipping signs, keep determinant of rot matrix same
		case 1:
			c->y = -c->y; c->z = -c->z;
			break;
		case 2:
			c->x = -c->x; c->z = -c->z;
			break;
		case 3:
			c->x = -c->x; c->y = -c->y;
			break;
		default:
			break;
	}
}

void rotate_coords(struct coord* coords, int nr_coords, int r) {
	int ii;
	for (ii = 0; ii < nr_coords; ++ii)
		rotate_coord(coords + ii, r);
}

int cmp_coord(const void* a, const void* b) {
	int d = ((struct coord*)a)->x - ((struct coord*)b)->x;
	if (d == 0)
		d = ((struct coord*)a)->y - ((struct coord*)b)->y;
	if (d == 0)
		d = ((struct coord*)a)->z - ((struct coord*)b)->z;
	return d;
}

bool compare_all_coordinates(struct scanner* scanner1, struct scanner* scanner2) {
	// contract: scanner1's coordinates are ordered by x-value already!
	// qsort(scanner1->coords, scanner1->nr_coords, sizeof(struct coord), cmp_coord);
	int rot;
	for (rot = 0; rot < 24; ++rot) {
		struct coord cpy[MAX_SCANCOORDS];
		memcpy(cpy, scanner2->coords, scanner2->nr_coords * sizeof(struct coord));
		rotate_coords(cpy, scanner2->nr_coords, rot);
		qsort(cpy, scanner2->nr_coords, sizeof(struct coord), cmp_coord);
		int idxa, idxb;
		for (idxa = 0; idxa < scanner1->nr_coords - 11; ++idxa) {
			for (idxb = 0; idxb < scanner2->nr_coords - 11; ++idxb) {
				// hypothesis: scanner1->coords[idxa] and scanner2->coords[idxb] are same point
				int dx = scanner1->coords[idxa].x - cpy[idxb].x;
				int dy = scanner1->coords[idxa].y - cpy[idxb].y;
				int dz = scanner1->coords[idxa].z - cpy[idxb].z;
				int idxaa = idxa + 1;
				int idxbb = idxb + 1;
				int count = 1;
				while (idxaa < scanner1->nr_coords && idxbb < scanner2->nr_coords) {
					int d = scanner1->coords[idxaa].x - (cpy[idxbb].x + dx);
					if (d == 0)
						d = scanner1->coords[idxaa].y - (cpy[idxbb].y + dy);
					if (d == 0)
						d = scanner1->coords[idxaa].z - (cpy[idxbb].z + dz);
					if (d == 0) {
						++count;
						++idxaa;
						++idxbb;
					}
					else if (d < 0)
						++idxaa;
					else
						++idxbb;
				}
				if (count >= 12) { // match found
					int ii;
					for (ii = 0; ii < scanner2->nr_coords; ++ii) {
						scanner2->coords[ii].x = cpy[ii].x + dx;
						scanner2->coords[ii].y = cpy[ii].y + dy;
						scanner2->coords[ii].z = cpy[ii].z + dz;
						scanner2->abs_coords = true;
					}
					return true;
				}
			}
		}
	}
	return false;
}

void print_scanner(struct scanner* scanners, int idx) {
	struct scanner* s = scanners + idx;
	printf("Scanner %d\n", idx);
	int ii;
	for (ii = 0; ii < s->nr_coords; ++ii) {
		printf("%d, %d, %d\n", s->coords[ii].x, s->coords[ii].y, s->coords[ii].z);
	}
}

int main(int argc, char* argv[]) {
	struct scanner scanners[MAX_SCANNERS];

	char *line = NULL;
	size_t len = 0;
	int scanner_idx = 0;
	int nr_scanners = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		if (line[1] == '-') {
			scanner_idx = atoi(line + 12);
			if (scanner_idx + 1 > nr_scanners)
				nr_scanners = scanner_idx + 1; // scanners start at 0
			scanners[scanner_idx].nr_coords = 0;
			scanners[scanner_idx].abs_coords = false;
		}
		else {
			struct coord c;
			sscanf(line, "%d,%d,%d", &c.x, &c.y, &c.z);
			scanners[scanner_idx].coords[scanners[scanner_idx].nr_coords++] = c;
		}
	}
	free(line);

	// Find connected scanners, make coordinates absolute in the process
	int scanner_stack[MAX_SCANNERS];
	int nr_scanners_on_stack = 0;

	int sa, sb;
	sa = 0;
	qsort(scanners[0].coords, scanners[0].nr_coords, sizeof(struct coord), cmp_coord);
	scanners[0].abs_coords = true;
	scanner_stack[nr_scanners_on_stack++] = 0;
	while (nr_scanners_on_stack) {
		sa = scanner_stack[--nr_scanners_on_stack];
		for (sb = 0; sb < nr_scanners; ++sb) {
			if (!scanners[sb].abs_coords) {
				if (compare_all_coordinates(&scanners[sa], &scanners[sb])) {
					// printf("%d -- %d\n", sa, sb);
					scanner_stack[nr_scanners_on_stack++] = sb;
				}
			}
		}
	}
	// Collect all coordinates
	int ii;
	struct coord allcoords[MAX_SCANNERS * MAX_SCANCOORDS];
	int nr_allcoords = 0;
	for (sa = 0; sa < nr_scanners; ++sa) {
		for (ii = 0; ii < scanners[sa].nr_coords; ++ii)
			allcoords[nr_allcoords++] = scanners[sa].coords[ii];
	}
	qsort(allcoords, nr_allcoords, sizeof(struct coord), cmp_coord);
	// count uniques
	struct coord last_coord = allcoords[0];
	int count = 1;
	for (ii = 1; ii < nr_allcoords; ++ii) {
		if (cmp_coord(&allcoords[ii], &last_coord) != 0) {
			++count;
			last_coord = allcoords[ii];
		}
	}
	printf("%d\n", count);

	return 0;
}
