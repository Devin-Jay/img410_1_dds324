#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "sgi.h"

#define errno_cleanup() if (errno != 0) goto cleanup;
#define errno_clear() errno = 0;

#ifndef ENOTSUP
#define ENOTSUP 4096
#endif

#ifndef EFTYPE
#define EFTYPE 4097
#endif

int get_byte(FILE *fh) {
  return fgetc(fh);
}

uint16_t get_short(FILE *fh) {
  uint8_t buffer[2];
  size_t cnt = fread(buffer, 2, 1, fh);
  return ((buffer[0] << 8) |
          (buffer[1]));
}

uint32_t get_long(FILE *fh) {
  uint8_t buffer[4];
  size_t cnt = fread(buffer, 4, 1, fh);
  return ((buffer[0] << 24) |
          (buffer[1] << 16) |
          (buffer[2] << 8) |
          (buffer[3]));
}

static void put_byte(FILE *fh, uint8_t val) {
  unsigned char buffer[1];
  buffer[0] = val;
  size_t cnt = fwrite(buffer, 1, 1, fh);
}

static void put_short(FILE *fh, uint16_t val) {
  unsigned char buffer[2];
  buffer[0] = (val>>8);
  buffer[1] = (val>>0);
  size_t cnt = fwrite(buffer, 2, 1, fh);
}

static void put_long(FILE *fh, uint32_t val) {
  unsigned char buffer[4];
  buffer[0] = (val>>24);
  buffer[1] = (val>>16);
  buffer[2] = (val>>8);
  buffer[3] = (val >> 0);
  size_t cnt = fwrite(buffer, 4, 1, fh);
}

void sgi_read(char *filename,
              uint8_t **pixmap,
              struct sgi_metadata *metadata) {

  errno_clear();

  FILE *fh = fopen(filename, "r");
  errno_cleanup();

  // IRIS image file magic number
  uint16_t magic = get_short(fh);
  errno_cleanup();

  if (magic != 474) {
    errno = EFTYPE;
    errno_cleanup();
  }

  // Storage format (rle=1; no rle=0)
  // rle not supported
  uint8_t storage = get_byte(fh);
  errno_cleanup();

  if (storage != 0) {
    // Unsupported storage type
    errno = ENOTSUP;
    errno_cleanup();
  }

  // Number of bytes per pixel channel
  // Only 1 or 2 is valid; 2 is unsupported here
  uint8_t bpc = get_byte(fh);
  errno_cleanup();

  if (bpc != 1) {
    // Unsupported bytes per channel
    errno = ENOTSUP;
    errno_cleanup();
  }

  // Number of dimensions
  // 1 = X x 1 x 1; unsupported
  // 2 = X x Y x 1; unsupported
  // 3 = X x Y x Z
  uint16_t dimension = get_short(fh);
  errno_cleanup();

  if (dimension != 3) {
    // Unsupported dimension type
    errno = ENOTSUP;
    return;
  }

  // X size in pixels
  metadata->width = get_short(fh);
  errno_cleanup();

  // Y size in pixels
  metadata->height = get_short(fh);
  errno_cleanup();

  // Number of channels
  // 1 = grayscale
  // 3 = RGB
  // 4 = RGBA
  metadata->channels = get_short(fh);
  errno_cleanup();

  // Minimum pixel value
  // Typically 0
  uint32_t pixmin = get_long(fh);
  errno_cleanup();

  // Maximum pixel value
  // Typically 255
  uint32_t pixmax = get_long(fh);
  errno_cleanup();

  if (pixmax > 255) {
    // Unsupported pixmax
    errno = ENOTSUP;
    errno_cleanup();
  }

  // Ignored
  // Should be zero; unchecked here
  uint32_t reserved_0 = get_long(fh);
  errno_cleanup();

  // Image name
  {
    char name[80] = "";
    size_t cnt = fread(name, 80, 1, fh);
    errno_cleanup();
  }

  // Colormap ID
  // 0 - normal behavior
  // 1 - dithered with 3 bit red, 3 bit green, 2 bit blue; unsupported
  // 2 - "screen" format that uses an indexed palette; unsupported
  // 3 - same as 2 with an SGI specific map?; unsupported
  uint32_t colormap = get_long(fh);
  errno_cleanup();

  if (pixmax > 255) {
    // Unsupported colormap
    errno = ENOTSUP;
    errno_cleanup();
  }

  // Ignored
  {
    char reserved_1[404];
    size_t cnt = fread(reserved_1, 404, 1, fh);
    errno_cleanup();
  }

  *pixmap = malloc(metadata->width * metadata->height * metadata->channels * 1);
  errno_cleanup();
  assert(*pixmap != NULL);

  for (int z = 0; z < metadata->channels; z += 1) {
    for (int y = metadata->height-1; y >= 0; y -= 1) {
      for (int x = 0; x < metadata->width; x += 1) {
        int c = get_byte(fh);
        errno_cleanup();

        int p = metadata->channels * (metadata->width * y + x) + z;
        (*pixmap)[p] = c;
      }
    }
  }

 cleanup:
  if (fh) {
    fclose(fh);
  }
  return;
}

void sgi_write(char *filename,
               uint8_t **pixmap,
               struct sgi_metadata *metadata) {

  FILE* fh = fopen(filename, "wb");
  errno_cleanup();

  // IRIS image file magic number
  put_short(fh, 474);
  errno_cleanup();

  // Storage format (rle=1; no rle=0)
  put_byte(fh, 0);
  errno_cleanup();

  // Number of bytes per pixel channel
  put_byte(fh, 1);
  errno_cleanup();

  // Number of dimensions
  put_short(fh, 3);
  errno_cleanup();

  // Dimensions
  put_short(fh, metadata->width);
  errno_cleanup();

  put_short(fh, metadata->height);
  errno_cleanup();

  put_short(fh, metadata->channels);
  errno_cleanup();

  // pixmin, pixmax
  put_long(fh, 0);
  errno_cleanup();

  put_long(fh, 255);
  errno_cleanup();

  // Reserved
  put_long(fh, 0);
  errno_cleanup();

  // Image name - unused
  for (int i=0; i<80; i+=1) {
    put_byte(fh, 0);
    errno_cleanup();
  }

  // Colormap ID
  put_long(fh, 0);
  errno_cleanup();

  // Reserved
  for (int i=0; i<404; i+=1) {
    put_byte(fh, 0);
    errno_cleanup();
  }

  for (int z = 0; z < metadata->channels; z += 1) {
    for (int y = metadata->height - 1; y >= 0; y -= 1) {
      for (int x = 0; x < metadata->width; x += 1) {
        int p = metadata->channels * (metadata->width * y + x) + z;
        put_byte(fh, (*pixmap)[p]);
        errno_cleanup();
      }
    }
  }

 cleanup:
  if (fh) {
    fclose(fh);
  }
  return;
}
