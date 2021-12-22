#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BOUNDARY 2

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
	img->pixels = NULL;
	img->capacity = 0;
	return img;
}

struct img* create_img_with_size(int w, int h) {
	struct img* img = mymalloc(sizeof(struct img));
	img->w = w;
	img->h = h;
	img->capacity = w * h;
	img->pixels = mycalloc(img->capacity, sizeof(char));
	return img;
}

void destroy_img(struct img* img) {
	if (img)
		free(img->pixels);
	free(img);
}

void img_set_pixel(struct img* img, int r, int c, bool v) {
	r = (r + img->h) % img->h;
	c = (c + img->w) % img->w;
	img->pixels[r * img->w + c] = (v ? 1 : 0);
}

bool img_get_pixel(struct img* img, int r, int c) {
	r = (r + img->h) % img->h;
	c = (c + img->w) % img->w;
	return img->pixels[r * img->w + c];
}

void img_add_row(struct img* img, char* line) {
	int c;
	if (img->w == 0) {
		int w = 0;
		while (line[w] == '.' || line[w] == '#')
			++w;
		img->w = w + 2 * BOUNDARY;
		img->capacity = img->w * img->w; // assume square
		img->pixels = mycalloc(img->capacity, sizeof(char));
		img->h = 2 * BOUNDARY;
	}
	if (img->w * img->h >= img->capacity) {
		printf("Reallocting mem\n");
		printf("img->w: %d, img->h: %d, cap: %d\n", img->w, img->h, img->capacity);
		img->capacity *= 2;
		img->pixels = myrealloc(img->pixels, img->capacity * sizeof(char));
		memset(img->pixels + img->capacity / 2, 0, img->capacity / 2 * sizeof(char));
	}
	int r = img->h - BOUNDARY;
	for (c = 0; c < BOUNDARY; ++c) {
		img_set_pixel(img, r, c, false);
		img_set_pixel(img, r, img->w - 1 - c, false);
	}
	for (c = 0; c < img->w - 2 * BOUNDARY; ++c)
		img_set_pixel(img, r, BOUNDARY + c, line[c] == '#');
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

struct img* process_img(struct img* img_in, struct algo* algo) {
	// create output img
	struct img* img_out = create_img_with_size(img_in->w + 2, img_in->h + 2); // conv adds 1 pixel on each edge
	int r, c; // refer to img_out
	for (r = 0; r < img_out->h; ++r) {
		for (c = 0; c < img_out->w; ++c) {
			int b = 0;
			int rr, cc;
			for (rr = -2; rr <= 0; ++rr) {
				for (cc = -2; cc <= 0; ++cc) {
					b = (b << 1) | (img_get_pixel(img_in, r + rr, c + cc) ? 1 : 0);
				}
			}
			img_set_pixel(img_out, r, c, algo->map[b]);
		}
	}

	return img_out;
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
	// show_img(img);

	int ii;
	for (ii = 0; ii < 50; ++ii) {
		struct img* img_out = process_img(img, algo);
		destroy_img(img);
		img = img_out;
		// show_img(img);
	}

	// count pixels
	int r, c;
	int count = 0;
	for (r = 0; r < img->h; ++r)
		for (c = 0; c < img->w; ++c)
			count += img_get_pixel(img, r, c) ? 1 : 0;
	printf("%d\n", count);
	destroy_img(img);

	free(algo);

	return 0;
}
