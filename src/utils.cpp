#include "utils.h"

// kernel
const int KERNEL[5][5] = {
    {1, 2, 3, 2, 1},
    {2, 4, 6, 4, 2},
    {3, 6, 9, 6, 3},
    {2, 4, 6, 4, 2},
    {1, 2, 3, 2, 1}
};

// sum of all kernel values
const int GSUM = 81;

// function to skip comments in PPM file
// loops until a non-comment character is found
void skip_comments(FILE *fh)
{
    int c;
    do
    {
        c = fgetc(fh);
        if (c == '#')
        {
            // skip entire comment line
            while ((c = fgetc(fh)) != '\n' && c != EOF);
        }
    } 
    while (c == '\n' || c == ' ' || c == '\t');

    ungetc(c, fh);
}

// reads input ppm file and returns PPMImage struct with data
PPMImage* readInputPPMFile(char *filename, int *width, int *height)
{
    FILE *fh = fopen(filename, "r");

    if (fh == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s for reading\n", filename);
        return NULL;
    }

    char magic[3];
    fscanf(fh, "%2s", magic);

    if (strcmp(magic, "P3") != 0)
    {
        fprintf(stderr, "Error: Unsupported PPM format %s. Only P3 format is supported.\n", magic);
        fclose(fh);
        return NULL;
    }

    skip_comments(fh);

    // read width, height, max color value
    int maxval;
    fscanf(fh, "%d %d", width, height);
    fscanf(fh, "%d", &maxval);

    PPMImage *img = (PPMImage*)malloc(sizeof(PPMImage));

    if (img == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for PPMImage struct\n");
        fclose(fh);
        return NULL;
    }

    img->width = *width;
    img->height = *height;
    img->pixels = (uint8_t*)malloc(3 * (*width) * (*height));

    if (img->pixels == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for PPMImage pixels\n");
        fclose(fh);
        free(img);
        return NULL;
    }

    int pixel_count = 3 * (*width) * (*height);

    // loop through pixel data
    // Read RGB values, ensure no invalid numbers
    for (size_t i = 0; i < pixel_count; i++)
    {
        skip_comments(fh);
        int val;
        if (fscanf(fh, "%d", &val) != 1) {
            fprintf(stderr, "Error: Unexpected end of file or invalid pixel at index %zu\n", i);
            free(img->pixels);
            fclose(fh);
            free(img);
            return NULL;
        }
        if (val < 0 || val > maxval) {
            fprintf(stderr, "Error: Pixel value %d out of range 0-%d at index %zu\n", val, maxval, i);
            free(img->pixels);
            fclose(fh);
            free(img);
            return NULL;
        }
        img->pixels[i] = (uint8_t)val;
    }

    fclose(fh);

    return img;
}

// function that applies filter to image and returns new PPMImage struct
PPMImage* applyFilter(PPMImage* img)
{
    // make new img
    PPMImage* newImg = (PPMImage*)malloc(sizeof(PPMImage));

    if (newImg == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for filtered PPMImage struct\n");
        return NULL;
    }

    newImg->width = img->width;
    newImg->height = img->height;
    newImg->pixels = (uint8_t*)malloc(3 * img->width * img->height);

    if (newImg->pixels == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for filtered PPMImage pixels\n");
        free(newImg);
        return NULL;
    }

    // loop through each pixel in the image
    for (int x = 0; x < img->width; x++)
    {
        for (int y = 0; y < img->height; y++)
        {
            // initialize weighted sum
            int weightedSum[3] = {0, 0, 0};

            // loop through kernel
            for (int kx = -2; kx <= 2; kx++)
            {
                for (int ky = -2; ky <= 2; ky++)
                {
                    // get neighbor pixel coordinates
                    int neighborX = x + kx;
                    int neighborY = y + ky;

                    // if neighbor is within bounds
                    if (neighborX >= 0 && neighborX < img->width && neighborY >= 0 && neighborY < img->height)
                    {
                        // apply kernel
                        weightedSum[0] += img->pixels[3 * (neighborY * img->width + neighborX) + 0] * KERNEL[kx + 2][ky + 2];
                        weightedSum[1] += img->pixels[3 * (neighborY * img->width + neighborX) + 1] * KERNEL[kx + 2][ky + 2];
                        weightedSum[2] += img->pixels[3 * (neighborY * img->width + neighborX) + 2] * KERNEL[kx + 2][ky + 2];
                    }
                    // otherwise, check if x is in bounds
                    else if (neighborX >= 0 && neighborX < img->width)  
                    {
                        // apply kernel with wrapped y
                        weightedSum[0] += img->pixels[3 * (neighborY * img->width + neighborX - img->width) + 0] * KERNEL[kx + 2][ky + 2];
                        weightedSum[1] += img->pixels[3 * (neighborY * img->width + neighborX - img->width) + 1] * KERNEL[kx + 2][ky + 2];
                        weightedSum[2] += img->pixels[3 * (neighborY * img->width + neighborX - img->width) + 2] * KERNEL[kx + 2][ky + 2];
                    }
                    // otherwise, check if y is in bounds
                    else if (neighborY >= 0 && neighborY < img->height)  
                    {
                        // apply kernel with wrapped x
                        weightedSum[0] += img->pixels[3 * (neighborY - img->width * img->width + neighborX) + 0] * KERNEL[kx + 2][ky + 2];
                        weightedSum[1] += img->pixels[3 * (neighborY - img->width * img->width + neighborX) + 1] * KERNEL[kx + 2][ky + 2];
                        weightedSum[2] += img->pixels[3 * (neighborY - img->width * img->width + neighborX) + 2] * KERNEL[kx + 2][ky + 2];
                    }
                    // otherwise, assume both are out of bounds
                    else
                    {
                        // apply kernel with wrapped x and y
                        weightedSum[0] += img->pixels[3 * (neighborY - img->width * img->width + neighborX - img->width) + 0] * KERNEL[kx + 2][ky + 2];
                        weightedSum[1] += img->pixels[3 * (neighborY - img->width * img->width + neighborX - img->width) + 1] * KERNEL[kx + 2][ky + 2];
                        weightedSum[2] += img->pixels[3 * (neighborY - img->width * img->width + neighborX - img->width) + 2] * KERNEL[kx + 2][ky + 2];
                    }
                }
            }

            // compute new pixel value
            newImg->pixels[3 * (y * newImg->width + x) + 0] = weightedSum[0] / GSUM;
            newImg->pixels[3 * (y * newImg->width + x) + 1] = weightedSum[1] / GSUM;
            newImg->pixels[3 * (y * newImg->width + x) + 2] = weightedSum[2] / GSUM;
        }
    }

    return newImg;
}

// function to write PPMImage struct to output ppm file
void writeOutputPPMFile(char *filename, PPMImage* img)
{
    // implementation for writing PPM file
    printf("Writing PPM file: %s\n", filename);
    FILE *fh = fopen(filename, "w");

    // write header
    fprintf(fh, "P3\n");
    fprintf(fh, "%d %d\n", img->width, img->height);
    fprintf(fh, "255\n");

    int pixel_count = 3 * img->width * img->height;

    // write pixel data
    for (size_t i = 0; i < pixel_count; i++)
    {
        fprintf(fh, "%d ", img->pixels[i]);
        if ((i + 1) % (img->width * 3) == 0)
        {
            fprintf(fh, "\n");
        }
    }

    fclose(fh);
}

