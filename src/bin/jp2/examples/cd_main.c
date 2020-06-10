#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <dirent.h>

#include <strings.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>

//#include "JPEG2K.h"
#include "opj_config.h"
#include <stdlib.h>
#include "openjpeg.h"
//#include "format_defs.h"

// Only needed for opj_string_s definition
//#include "opj_string.h"

int Compress(OPJ_BYTE *, size_t, OPJ_BYTE **, size_t *);
int Decompress(OPJ_BYTE *, size_t, OPJ_BYTE **, size_t *);

OPJ_BYTE *read_image(const char* filename, size_t *num_bytes)
{

   int header[3];
   FILE *infile;
   infile = fopen(filename, "rb"); // r for read, b for binary
   fread(header, sizeof(int), 3, infile);
   fclose(infile);
   size_t nb = (header[0] * header[1] * header[2] + 3) * sizeof(int);
   printf("Read image: %s   (%d x %d)\n", filename, header[1], header[2]);

   OPJ_BYTE *image_buffer = (char *)malloc(nb);
   infile = fopen(filename, "rb"); // r for read, b for binary
   fread(image_buffer, nb, 1, infile);
   fclose(infile);

   *num_bytes = nb;
   return image_buffer;
}

// Black, Red, Orange, Pink, White, Yellow, Purple, Blue, Green
   int R[9] = {  0, 201, 234, 233, 255, 255, 101,  12,   0};
   int G[9] = {  0,  23,  85,  82, 255, 234,  49,   2,  85};
   int B[9] = {  0,  30,   6, 149, 255,   0, 142, 196,  46};

OPJ_BYTE *build_image(size_t num_comps, size_t width, size_t height)
{
   // Generates palete of 3x3 solid colors (see above) as int32 pixel image
   // As written here, Compress requires header of 3 ints; Decompress
   // returns binary pixel values with header
   size_t buffer_size = (num_comps * width * width + 3) * sizeof(OPJ_INT32);
   OPJ_BYTE *image_buffer = (OPJ_BYTE *)malloc(buffer_size);
   int *l_data = (int*)image_buffer;

   // Insert "header" info into image buffer
   int *header = (int *)image_buffer;
   header[0] = num_comps;
   l_data++;
   header[1] = height;
   l_data++;
   header[2] = width;
   l_data++;

   int p = 0, q = width*height, r = 2*width*height;
   int c, i, j, k;
   for (c = 0; c < 9; c += 3) {
      for (i = 0; i < 100; ++i) {
         for (j = c; j < c+3; ++j) {
            for (k = 0; k < 100; ++k) {
               l_data[p++] = R[j];
               l_data[q++] = G[j];
               l_data[r++] = B[j];
            }
         }
      }
   }

   return image_buffer;
}

int main(int argc, char **argv)
{
   OPJ_BYTE* image_buffer;       // image buffer of raw pixels
   OPJ_BYTE* compressed_buffer;  // compression results
   size_t buffer_size = 0;       // number of bytes in raw pixel buffer
   size_t compressed_size = 0;   // number of bytes in buffer from compress

   char JP2fileName[20] = "encode_image.jp2";
   char PIXfileName[20] = "decode_image.pxl";
   
   size_t numcomps = 3;
   size_t image_height = 300;
   size_t image_width = 300;
   image_buffer = build_image(numcomps, image_width, image_height);
   buffer_size = (numcomps * image_height * image_width + 3) * sizeof(OPJ_INT32);

   Compress(image_buffer, buffer_size, &compressed_buffer, &compressed_size);

   free(image_buffer);

/* Can the image_buffer be output as a binary file recognized as JP2? */
   FILE *write_ptr;
   write_ptr = fopen(JP2fileName,"wb");  // w for write, b for binary
   fwrite(compressed_buffer, compressed_size, 1, write_ptr);
   fclose(write_ptr);

   Decompress(compressed_buffer, compressed_size, &image_buffer, &buffer_size);

/* Does the image_buffer hold the pixel values that went into the tile? */
   write_ptr = fopen(PIXfileName,"wb");
   fwrite(image_buffer, buffer_size, 1, write_ptr);
   fclose(write_ptr);

   printf("\nDONE\n");
}
