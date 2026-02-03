#include <stdio.h>
#include <stdint.h>

// Don't do this:
extern "C" {
	#include "sgi.h"
}


void normalize(uint8_t* pixmap, int width, int height, int channels) {

	uint8_t C_min = 255;
	uint8_t C_max = 0;

	// Find C_min and C_max in the image.
	for (int i=0; i<width*height*channels; i++) {
		uint8_t C_i = pixmap[i];
		if (C_i < C_min) {
			C_min = C_i;
		}
		if (C_i > C_max) {
			C_max = C_i;
		}
	}

	// Adjust the image.
	float scale = 255.0 / (C_max - C_min);
	for (int i=0; i<width*height*channels; i++) {
		pixmap[i] = (pixmap[i] - C_min) * scale;
	}
	printf("C_min=%d, C_max=%d\n", C_min, C_max);
}


int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Usage: normalzie input.sgi output.sgi\n");
		return 1;
	}

	// 1. Read the file to filter.

	sgi_metadata metadata;
	uint8_t *pixmap;

	sgi_read(argv[1], &pixmap, &metadata);

	// 2. Filter it.

	normalize(pixmap, metadata.width, metadata.height, metadata.channels);

	// 3. Write the filtered file.

	sgi_write(argv[2], &pixmap, &metadata);

}


