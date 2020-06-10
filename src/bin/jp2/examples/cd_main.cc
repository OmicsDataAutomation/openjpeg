/**
 * @file cd_main.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2019-2020 Omics Data Automation, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION Driver code to create int32 palette image, compress
 *     the image to memory, write the compressed image to a file (for manual
 *     verification), decompress the image from memory, and print the
 *     decompressed pixel values to a file (for verification).
 *
 *     While not part of the image, 3 integer "header" values are added as a
 *     prefix to the pixel data before sending the image to Compress. These
 *     bytes are read in compress to set parameter values of the number of
 *     components in the image, and the width and height of the image. These
 *     header bytes are stripped off the image before compression.
 *
 *     Before return from Decompress, the header bytes are restored as a 
 *     preface to the returned pixel values.
 */

#include <stdio.h>
#include "opj_config.h"
#include <stdlib.h>
#include "openjpeg.h"

int Compress(OPJ_BYTE *, size_t, OPJ_BYTE **, size_t *);
int Decompress(OPJ_BYTE *, size_t, OPJ_BYTE **, size_t *);

// Memory cleanup function called from Compress and Decompress
void cleanup(void *l_data, opj_stream_t *l_stream, opj_codec_t *l_codec, opj_image_t *l_image)
{
   if (l_data)   free(l_data);
   if (l_stream) opj_stream_destroy(l_stream);
   if (l_codec)  opj_destroy_codec(l_codec);
   if (l_image)  opj_image_destroy(l_image);
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

   if ((width % 3 != 0) && (height % 3 != 0)) {
      printf("Image width and height values must be divisible by 3\n");
      exit(1);
   }
   // Insert "header" info into image buffer
   int *header = (int *)image_buffer;
   header[0] = num_comps; l_data++;
   header[1] = width;     l_data++;
   header[2] = height;    l_data++;

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

   free(compressed_buffer);
   free(image_buffer);

   printf("\nDONE\n");
}
