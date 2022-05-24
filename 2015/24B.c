#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>

#define MAX_PACKAGES 100

struct package {
	int weight;
	int group; // 0: unassigned
};

struct group1score {
	int nrpackages;
	int64_t qe; // product of weights
};

void print_package_group(struct package* p, int n, int group) {
	int sum = 0;
	int64_t prod = 1;
	bool first = true;
	for (int ii = 0; ii < n; ++ii) {
		if (p[ii].group == group) {
			if (!first)
				printf(" + ");
			printf("%3d", p[ii].weight);
			first = false;
			sum += p[ii].weight;
			prod *= p[ii].weight;
		}
	}
	printf(" = %d (qe = %" PRId64 ")\n", sum, prod);
}

int64_t calc_qe(struct package* p, int n) {
	int64_t prod = 1;
	for (int ii = 0; ii < n; ++ii)
		if (p[ii].group == 1)
			prod *= p[ii].weight;
	return prod;
}

bool check_group23(struct package* p, int nr_packages, int idx0, int sum2_left, int sum3_left) {
	if (sum2_left == 0 && sum3_left == 0) return true;
	if (sum2_left < 0 || sum3_left < 0) return false;
	if (idx0 >= nr_packages) return false;
	if (p[idx0].weight > sum2_left && p[idx0].weight > sum3_left) return false; // nrs are sorted

	if (check_group23(p, nr_packages, idx0 + 1, sum2_left, sum3_left)) // p[idx0] --> group 4
		return true;
	if (check_group23(p, nr_packages, idx0 + 1, sum2_left, sum3_left - p[idx0].weight)) // p[idx0] --> group 3
		return true;
	return check_group23(p, nr_packages, idx0 + 1, sum2_left - p[idx0].weight, sum3_left); // p[idx0] --> group 2
}

void find_group1(struct package* p, int nr_packages, int idx0, int sum, int target_sum, int used, struct group1score* best_score) {
	if (sum == target_sum && used <= best_score->nrpackages) { // found an option
		if (check_group23(p, nr_packages, 0, target_sum, target_sum)) {
			int64_t qe = calc_qe(p, nr_packages);
			if (used < best_score->nrpackages || qe < best_score->qe) {
				// print_package_group(p, nr_packages, 1);
				best_score->nrpackages = used;
				best_score->qe = qe;
			}
		}
	}
	if (idx0 >= nr_packages) return; // prune: out of packages
	if (p[idx0].weight + sum > target_sum) return; // prune: cannot reach target (weights are sorted)
	if (used > best_score->nrpackages) return; // prune: not the smallest selection
	if (used == best_score->nrpackages && sum < target_sum) return; // prune: will not be smallest selection

	// option 1: we don't use p[idx0] in group1
	p[idx0].group = 0;
	find_group1(p, nr_packages, idx0 + 1, sum, target_sum, used, best_score);
	// option 2: we use p[idx0] in group1
	p[idx0].group = 1;
	find_group1(p, nr_packages, idx0 + 1, sum + p[idx0].weight, target_sum, used + 1, best_score);

	p[idx0].group = 0; // reset package use before returning
}

int main(int argc, char* argv[]) {
	struct package p[MAX_PACKAGES];
	int nr_packages = 0;

	// read weights
	int x;
	while (scanf("%d", &x) == 1) {
		p[nr_packages].weight = x;
		p[nr_packages].group = 0; // unassigned
		++nr_packages;
	}

	int sum = 0;
	for (int ii = 0; ii < nr_packages; ++ii)
		sum += p[ii].weight;
	if (sum % 4 != 0) {
		fprintf(stderr, "Sum not divisible by 4!\n");
		return 1;
	}
	sum /= 4;

	struct group1score best_score;
	best_score.nrpackages = INT_MAX;
	best_score.qe = LLONG_MAX;
	find_group1(p, nr_packages, 0, 0, sum, 0, &best_score);
	printf("%" PRId64 "\n", best_score.qe);

	return 0;
}
