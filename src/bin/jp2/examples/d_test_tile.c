#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <dirent.h>

#include <strings.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>

#include "opj_config.h"
#include "openjpeg.h"
#include "format_defs.h"

/**
 * sample error callback expecting a FILE* client object
 * */
static void error_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[ERROR] %s", msg);
}
/**
 * sample warning callback expecting a FILE* client object
 * */
static void warning_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[WARNING] %s", msg);
}
/**
 * sample debug callback expecting no client object
 * */
static void info_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[INFO] %s", msg);
}


void copy_pixels_out(OPJ_UINT32 numcomps, OPJ_UINT32 image_height, OPJ_UINT32 image_width, OPJ_BYTE * l_data, OPJ_UINT32 l_data_size, char** return_buff, size_t *size_out)
{
   size_t image_bytes = 3*sizeof(OPJ_UINT32) + l_data_size;
//printf("  copy_pixels_out: image_bytes == %d\n", image_bytes);

   char *out_buff = (char *) malloc(image_bytes * sizeof(char));
   if (!(out_buff)) {
     fprintf(stderr, "Fail to allocate %d bytes: copy_pixels_out\n", image_bytes);
     return;
   }
   OPJ_UINT32 *buffer = (OPJ_UINT32 *) out_buff;

   buffer[0] = numcomps;      // number of color components
   buffer[1] = image_height;  // image height
   buffer[2] = image_width;   // image width

   char *ob = out_buff;
   ob += 3*sizeof(OPJ_UINT32);

   memcpy(ob, l_data, l_data_size);
   
   *return_buff = out_buff;
   *size_out = image_bytes;
}

void cleanup(void *l_data, opj_stream_t *l_stream, opj_codec_t *l_codec, opj_image_t *l_image)
{
   if (l_data)   free(l_data);
   if (l_stream) opj_stream_destroy(l_stream);
   if (l_codec)  opj_destroy_codec(l_codec);
   if (l_image)  opj_image_destroy(l_image);
}

int Decompress(char* buf_in, size_t size_in, char **tile_out, size_t *size_out) 
{
   opj_dparameters_t l_param;
   opj_codec_t * l_codec;
   opj_image_t * l_image;
   opj_stream_t * l_stream;
   OPJ_UINT32 l_data_size;
   OPJ_UINT32 l_start_data_size = 1000;
   OPJ_BYTE * l_data = (OPJ_BYTE *) malloc(l_start_data_size);

   OPJ_UINT32 l_tile_index;
   OPJ_INT32 l_current_tile_x0;
   OPJ_INT32 l_current_tile_y0;
   OPJ_INT32 l_current_tile_x1;
   OPJ_INT32 l_current_tile_y1;
   OPJ_UINT32 l_nb_comps=0;
   OPJ_BOOL l_go_on = OPJ_TRUE;

   OPJ_UINT32 image_height;
   OPJ_UINT32 image_width;
   OPJ_UINT32 numcomps;

   /* Set the default decoding parameters */
   opj_set_default_decoder_parameters(&l_param);
   l_param.decod_format = JP2_CFMT;

   /** you may here add custom decoding parameters */
   /* do not use layer decoding limitations */
   l_param.cp_layer = 0;

   /* do not use resolutions reductions */
   l_param.cp_reduce = 0;

   /** Fixed to match Compress format **/
   l_codec = opj_create_decompress(OPJ_CODEC_JP2);
   if (!l_codec){
      fprintf(stderr, "ERROR -> d_test: failed to create decompress codec\n");
      cleanup(l_data, NULL, NULL, NULL);
      return EXIT_FAILURE;
   }

   /** Setup user data into stream **/
   l_stream = opj_stream_create_memory_stream((void *)buf_in, size_in, OPJ_TRUE);
   if (!l_stream){
      fprintf(stderr, "ERROR -> d_test: failed to create the memory stream\n");
      cleanup(l_data, NULL, l_codec, NULL);
      return EXIT_FAILURE;
   }

   /* Setup the decoder decoding parameters using user parameters */
   if (! opj_setup_decoder(l_codec, &l_param)) {
      fprintf(stderr, "ERROR -> d_test: failed to setup the decoder\n");
      cleanup(l_data, l_stream, l_codec, NULL);
      return EXIT_FAILURE;
   }

   /* catch events using our callbacks and give a local context */
//   opj_set_info_handler(l_codec, info_callback,00);
//   opj_set_warning_handler(l_codec, warning_callback,00);
//   opj_set_error_handler(l_codec, error_callback,00);

   /* Read the main header of the codestream and if necessary the JP2 boxes*/
   if (! opj_read_header(l_stream, l_codec, &l_image)) {
      fprintf(stderr, "ERROR -> d_test: failed to read the header\n");
      cleanup(l_data, l_stream, l_codec, NULL);
      return EXIT_FAILURE;
   }

// PULL OUT numcomps, image_height, image_width from  l_stream
   image_height = l_image->comps[0].h;
   image_width = l_image->comps[0].w;
   numcomps = l_image->numcomps;

   if (!opj_set_decode_area(l_codec, l_image, 0, 0, 0, 0)){ // whole image
      fprintf(stderr, "ERROR -> d_test: failed to set the decoded area\n");
      cleanup(l_data, l_stream, l_codec, l_image);
      return EXIT_FAILURE;
   }

   if (! opj_read_tile_header( l_codec,
                               l_stream,
                               &l_tile_index,
                               &l_data_size,
                               &l_current_tile_x0,
                               &l_current_tile_y0,
                               &l_current_tile_x1,
                               &l_current_tile_y1,
                               &l_nb_comps,
                               &l_go_on)) {
      fprintf(stderr, "ERROR -> d_test: failed to set read tile header\n");
      cleanup(l_data, l_stream, l_codec, l_image);
      return EXIT_FAILURE;
   }

   if (l_go_on) {
      //printf("d_test: l_data_size .GT. %d  %d\n", l_data_size, l_start_data_size);
      if (l_data_size > l_start_data_size) {
         OPJ_BYTE *l_new_data = (OPJ_BYTE *) realloc(l_data, l_data_size+12);
         if (! l_new_data) {
            cleanup(l_data, l_stream, l_codec, l_image);
            return EXIT_FAILURE;
         }
         l_data = l_new_data;
         l_start_data_size = l_data_size;
      }

      if (! opj_decode_tile_data(l_codec,l_tile_index,l_data,l_data_size,l_stream)) {
         cleanup(l_data, l_stream, l_codec, l_image);
         return EXIT_FAILURE;
      }
      /** now should inspect image to know the reduction factor and then how to behave with data */
   } else {
      fprintf(stderr, "ERROR -> d_test: Current tile number and total number of tiles problem\n");
      cleanup(l_data, l_stream, l_codec, l_image);
      return EXIT_FAILURE;
   }

   if (! opj_end_decompress(l_codec,l_stream)) {
      fprintf(stderr, "ERROR -> d_test: Unable to end decompress correctly\n");
      cleanup(l_data, l_stream, l_codec, l_image);
      return EXIT_FAILURE;
   }

   copy_pixels_out(numcomps, image_height, image_width, l_data, l_data_size, tile_out, size_out);

   /* Free memory */
   cleanup(l_data, l_stream, l_codec, l_image);

   return EXIT_SUCCESS;
}
