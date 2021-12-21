#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void* mymalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

void* mycalloc(size_t nmemb, size_t size) {
	void* p = calloc(nmemb, size);
	if (!p)
		fprintf(stderr, "could not allocate %zu bytes of memory\n", size);
	return p;
}

void* myrealloc(void* p, size_t size) {
	p = realloc(p, size);
	if (!p)
		fprintf(stderr, "Could not reallocate %zu bytes of memory\n", size);
	return p;
}

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

struct img {
	int w;
	int h;
	int capacity;
	char* pixels;
};

struct algo {
	char map[512];
};

struct algo* line_to_algo(char* line) {
	struct algo* a = mymalloc(sizeof(struct algo));
	int ii;
	for (ii = 0; ii < 512; ++ii)
		a->map[ii] = line[ii] == '#' ? 1 : 0;
	return a;
}

struct img* create_img() {
	struct img* img = mymalloc(sizeof(struct img));
	img->w = 0;
	img->h = 0;
	img->pixels = 0;
	img->capacity = 0;
	return img;
}

void img_set_pixel(struct img* img, int r, int c, bool v) {
	img->pixels[(r + 1) * (img->w + 2) + c + 1] = (v ? 1 : 0);
}

bool img_get_pixel(struct img* img, int r, int c) {
	return img->pixels[(r + 1) * (img->w + 2) + c + 1];
}

void img_add_row(struct img* img, char* line) {
	int c;
	if (img->w == 0) {
		int w = 0;
		while (line[w] == '.' || line[w] == '#')
			++w;
		img->w = w;
		img->capacity = (img->w + 2) * (img->w + 2); // boundary around image
		img->pixels = mycalloc(img->capacity, sizeof(char)); // assume sqr
		img->h = 0;
	}
	if ((img->w + 2) * (img->h + 2) >= img->capacity) {
		img->capacity *= 2;
		img->pixels = myrealloc(img->pixels, img->capacity * sizeof(char));
		memset(img->pixels + img->capacity / 2, 0, img->capacity / 2 * sizeof(char));
	}
	img_set_pixel(img, img->h, -1, false);
	for (c = 0; c < img->w; ++c)
		img_set_pixel(img, img->h, c, line[c] == '#');
	img_set_pixel(img, img->h, img->w, false);
	++img->h;
}

void show_img(struct img* img) {
	int r,c;
	for (r = 0; r < img->h; ++r) {
		for (c = 0; c < img->w; ++c) {
			putchar(img_get_pixel(img, r, c) ? '#' : '.');
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[]) {
	struct img* img = create_img();

	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	struct algo* algo = line_to_algo(line);

	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		img_add_row(img, line);
	}
	free(line);

	show_img(img);

	free(img);
	free(algo);

	return 0;
}
