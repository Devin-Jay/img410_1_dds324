// header files
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstdint>

typedef struct {
    int width;
    int height;
    uint8_t *pixels; // RGB interleaved
} PPMImage;

// function declarations
PPMImage* readInputPPMFile(char *filename, int *width, int *height);
void skip_comments(FILE *fh);
PPMImage* applyFilter(PPMImage* img);
void writeOutputPPMFile(char *filename, PPMImage* img);