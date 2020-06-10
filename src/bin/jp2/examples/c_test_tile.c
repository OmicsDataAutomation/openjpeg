#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "opj_config.h"
#include "openjpeg.h"

#include "stdlib.h"

static int tile_num = 0;
/* -------------------------------------------------------------------------- */

/**
sample error debug callback expecting no client object
*/
static void error_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning debug callback expecting no client object
*/
static void warning_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[INFO] %s", msg);
}

static INLINE OPJ_UINT32 opj_uint_max(OPJ_UINT32  a, OPJ_UINT32  b)
{
    return (a > b) ? a : b;
}

static INLINE OPJ_UINT32 opj_uint_min(OPJ_UINT32  a, OPJ_UINT32  b)
{
    return (a < b) ? a : b;
}

/* -------------------------------------------------------------------------- */

void get_header_values(char *tile_in,
                       OPJ_UINT32 *nc,
                       int *ih, int *iw)
{
   int *buffer = (int *) tile_in;

   *nc = buffer[0];       // number of color components
   *ih = buffer[1];       // image height
   *iw = buffer[2];       // image width

   return;
}

#define NUM_COMPS_MAX 4

int Compress(char *tile_in, size_t tile_size_in, char** buff_out, size_t* size_out )
{
    opj_cparameters_t l_param;
    opj_codec_t * l_codec;
    opj_image_t * l_image;
    opj_image_cmptparm_t l_image_params[NUM_COMPS_MAX];
    opj_stream_t * l_stream;
    OPJ_UINT32 l_nb_tiles_width, l_nb_tiles_height, l_nb_tiles;
    OPJ_UINT32 l_data_size;
    size_t len;

    opj_image_cmptparm_t * l_current_param_ptr;
    OPJ_UINT32 i;
    OPJ_BYTE *l_data;

    OPJ_UINT32 num_comps;
    int image_width;
    int image_height;
    int tile_width;
    int tile_height;
  
// Fixed parameter values. May need to amend if different image types are used
    int comp_prec = 8;
    int irreversible = 1;
    int cblockw_init = 64;
    int cblockh_init = 64;
    int numresolution = 6;
    OPJ_UINT32 offsetx = 0;
    OPJ_UINT32 offsety = 0;
    int quality_loss = 0; // lossless

    get_header_values(tile_in, &num_comps, &image_height, &image_width);

// Use whole "image" as single tile; tiling set in TileDB level
    tile_height = image_height;
    tile_width = image_width;

    opj_set_default_encoder_parameters(&l_param); // Default parameters

    if (num_comps > NUM_COMPS_MAX) {
        return 1;
    }

//  Set data pointer to the first pixel byte beyond the header values
    OPJ_UINT32 header_offset = 3 * sizeof(int);
    l_data = (OPJ_BYTE*) &tile_in[header_offset];
    OPJ_UINT32 tilesize = tile_size_in - header_offset;

    /* tile definitions parameters */
    /* position of the tile grid aligned with the image */
    l_param.cp_tx0 = 0;
    l_param.cp_ty0 = 0;
    /* tile size, we are using tile based encoding */
    l_param.tile_size_on = OPJ_TRUE;
    l_param.cp_tdx = tile_width;
    l_param.cp_tdy = tile_height;

    /* code block size */
    l_param.cblockw_init = cblockw_init;
    l_param.cblockh_init = cblockh_init;

    /* use irreversible encoding ?*/
    l_param.irreversible = irreversible;

    /** number of resolutions */
    l_param.numresolution = numresolution;

    /** progression order to use*/
    l_param.prog_order = OPJ_LRCP; // default

    /* image definition */
    l_current_param_ptr = l_image_params;
    for (i = 0; i < num_comps; ++i) {
        /* do not bother bpp useless */
        /*l_current_param_ptr->bpp = COMP_PREC;*/
        l_current_param_ptr->dx = 1;
        l_current_param_ptr->dy = 1;

        l_current_param_ptr->h = (OPJ_UINT32)image_height;
        l_current_param_ptr->w = (OPJ_UINT32)image_width;

        l_current_param_ptr->sgnd = 0;
        l_current_param_ptr->prec = (OPJ_UINT32)comp_prec;

        l_current_param_ptr->x0 = offsetx;
        l_current_param_ptr->y0 = offsety;

        ++l_current_param_ptr;
    }

    /* should we do j2k or jp2 ?*/
    l_codec = opj_create_compress(OPJ_CODEC_JP2);
    // l_codec = opj_create_compress(OPJ_CODEC_J2K);

    if (!l_codec) {
        return 1;
    }

    /* catch events using our callbacks and give a local context */
//    opj_set_info_handler(l_codec, info_callback, 00);
//    opj_set_warning_handler(l_codec, warning_callback, 00);
//    opj_set_error_handler(l_codec, error_callback, 00);

    if (num_comps == 3)
       l_image = opj_image_tile_create(num_comps, l_image_params, OPJ_CLRSPC_SRGB);
    else if (num_comps == 1)
       l_image = opj_image_tile_create(num_comps, l_image_params, OPJ_CLRSPC_GRAY);
    else 
       l_image = opj_image_tile_create(num_comps, l_image_params, OPJ_CLRSPC_UNKNOWN);
    if (! l_image) {
        //free(l_data);
        opj_destroy_codec(l_codec);
        return 1;
    }

    l_image->x0 = offsetx;
    l_image->y0 = offsety;
    l_image->x1 = offsetx + (OPJ_UINT32)image_width;
    l_image->y1 = offsety + (OPJ_UINT32)image_height;
    if (num_comps == 3)
       l_image->color_space = OPJ_CLRSPC_SRGB;
    else if (num_comps == 1)
       l_image->color_space = OPJ_CLRSPC_GRAY;
    else 
       l_image->color_space = OPJ_CLRSPC_UNKNOWN;

    if (! opj_setup_encoder(l_codec, &l_param, l_image)) {
        fprintf(stderr, "ERROR -> c_test: failed to setup the codec!\n");
        opj_destroy_codec(l_codec);
        opj_image_destroy(l_image);
        //free(l_data);
        return 1;
    }

    l_stream = (opj_stream_t *) opj_stream_create_default_memory_stream(OPJ_FALSE);
    if (! l_stream) {
        fprintf(stderr,
                "ERROR -> c_test: failed to create the memory stream!\n");
        opj_destroy_codec(l_codec);
        opj_image_destroy(l_image);
        //free(l_data);
        return 1;
    }

    if (! opj_start_compress(l_codec, l_image, l_stream)) {
        fprintf(stderr, "ERROR -> c_test: failed to start compress!\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(l_image);
        //free(l_data);
        return 1;
    }

/*
    printf("$$$ After start_compress\n");
    opj_stream_private_t * p_stream;
    mem_stream_t * my_data;
    p_stream = (opj_stream_private_t *) l_stream;
    my_data = (mem_stream_t*) p_stream->m_user_data;

   printf ("Print first %d bytes in compressed buffer:\n", p_stream->m_bytes_in_buffer);
   int j;
   OPJ_BYTE *t;
   t = &(my_data->mem_data[0]);
   for (j = 0; j < p_stream->m_bytes_in_buffer ; ++j) {
      if (!(j%16)) printf("\n");
      printf("%02x ",t[j]);
   }
   printf("\n");
*/

   /** Always write tile 0 **/
    if (! opj_write_tile(l_codec, 0, l_data, tilesize, l_stream)) {
       fprintf(stderr, "ERROR -> test_tile_encoder: failed to write the tile %d!\n", i);
       opj_stream_destroy(l_stream);
       opj_destroy_codec(l_codec);
       opj_image_destroy(l_image);
       //free(l_data);
       return 1;
    }

/*
    ++tile_num;
    p_stream = (opj_stream_private_t *) l_stream;
    my_data = (mem_stream_t*) p_stream->m_user_data;
   printf ("\nBEFORE opj_end_compress\nPrint first %d bytes in compressed buffer:\n", p_stream->m_bytes_in_buffer);
   t = &(my_data->mem_data[0]);
   for (j = 0; j < p_stream->m_bytes_in_buffer ; ++j) {
      if (!(j%16)) printf("\n");
      printf("%02x ",t[j]);
   }
   printf("\n");
*/


    if (! opj_end_compress(l_codec, l_stream)) {
        fprintf(stderr, "ERROR -> test_tile_encoder: failed to end compress!\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(l_image);
        //free(l_data);
        return 1;
    }

    *buff_out = opj_mem_stream_copy(l_stream, size_out);

    opj_stream_destroy(l_stream);
    opj_destroy_codec(l_codec);
    opj_image_destroy(l_image);

    return 0;
}
