#include <cstdio>
#include "utils.h"


int main(int argc, char* argv[])
{
    int width, height;

    // Handle improper arguments
    if (argc < 3) {
        fprintf(stderr, "Error: Missing input/output filenames\n");
        return 1;
    }

	// read input file and store in PPMImage struct
    PPMImage* img = readInputPPMFile(argv[1], &width, &height);

    // perform filter operation
    PPMImage* filtered_img = applyFilter(img);

    // write new image to output file
    writeOutputPPMFile(argv[2], filtered_img);

    // free allocated memory
    free(img->pixels);
    free(img);
    free(filtered_img->pixels);
    free(filtered_img);

	return 0;
}