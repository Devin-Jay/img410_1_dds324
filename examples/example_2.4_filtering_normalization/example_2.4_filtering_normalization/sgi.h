typedef struct sgi_metadata {
  uint16_t width;
  uint16_t height;
  uint16_t channels;
  uint8_t bpc;
} sgi_metadata;

void sgi_write(char *filename,
               uint8_t **pixmap,
               sgi_metadata *metadata);

void sgi_read(char *filename,
              uint8_t **pixmap,
              sgi_metadata *metadata);
