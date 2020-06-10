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

void print_POC(opj_poc_t POC)
{

/**
 * Progression order changes
 *
 */
    printf("      Res num start, Cmpnt num start: %d %d\n", POC.resno0, POC.compno0);
    printf("      Lyr num end, Res num end, Cmpnt num end: %d %d %d\n", POC.layno1, POC.resno1, POC.compno1);
    printf("      Lyr num start, POCrnct num start, POCrnct num end: %d %d %d\n", POC.layno0, POC.precno0, POC.precno1);
    printf("      Progression order enum: %d %d\n", POC.prg1, POC.prg);
    printf("      Progression order string: %s\n", POC.progorder);
    printf("      Tile number: %u\n", POC.tile);
    printf("      Start and end values for Tile width and height: %d %d  %d %d\n", POC.tx0, POC.tx1, POC.ty0, POC.ty1);
    printf("      Start value, initialised in pi_initialise_encode: %d %d %d %d\n", POC.layS, POC.resS, POC.compS, POC.prcS);
    printf("      End value, initialised in pi_initialise_encode: %d %d %d %d\n", POC.layE, POC.resE, POC.compE, POC.prcE);
    printf("      Start and end values of Tile width and height: %d %d  %d %d  %d %d\n", POC.txS, POC.txE, POC.tyS, POC.tyE, POC.dx, POC.dy);
    printf("      Temporary values for Tile parts: %u %u %u %u %u %u\n", POC.lay_t, POC.res_t, POC.comp_t, POC.prc_t, POC.tx0_t, POC.ty0_t);

}

void print_parameters(opj_cparameters_t P)
{
   int i;

   printf("-- Compression Parameters: ------------------------------------------------------\n");
   printf("  Tile Size On? : %d\n", P.tile_size_on);
   printf("  XTOsiz, YTOsiz, XTsiz, YTsiz: %d %d %d %d\n", P.cp_tx0, P.cp_ty0, P.cp_tdx, P.cp_tdy);
   printf("  Allocation by rate/distortion: %d\n", P.cp_disto_alloc);
   printf("  Allocation by fixed layer: %d\n", P.cp_fixed_alloc);
   printf("  Add fixed_quality: %d\n", P.cp_fixed_quality);
   printf("  Fixed layer (cp_matrice array)\n");
   if (! P.cp_matrice)
      printf("     NULL\n");
   else {
      printf("     ");
      for (i = 0; i < 4; ++i)
         printf("%d ", P.cp_matrice[i]);
      printf("\n");
   }
   printf("  Comment for coding: %s\n", P.cp_comment);
   printf("  csty : Coding style: %d\n", P.csty);
   printf("  Progression order (default OPJ_LRCP): %d\n", P.prog_order);
   printf("  Number of POC, default to 0: %u\n", P.numpocs);
   if (P.numpocs > 0) {
      for (i = 0; i < P.numpocs; ++i) {
         printf("    Progression order change[%d]:\n", i);
         print_POC(P.POC[i]);
      }
   }
   printf("  Number of layers: %d\n", P.tcp_numlayers);
   if (P.tcp_numlayers > 0) {
      for (i = 0; i < P.tcp_numlayers; ++i)
         printf("    TCP Rates[%d]:  %f\n", i, P.tcp_rates[i]);
   }
   if (P.tcp_numlayers > 0) {
      for (i = 0; i < P.tcp_numlayers; ++i)
         printf("    TCP Distoratio[%d]:  %f\n", i, P.tcp_distoratio[i]);
   }
   printf("\n  Number of resolutions: %d\n", P.numresolution);
   printf("  Initial code block width, height (default 64):  %d  %d\n", P.cblockw_init, P.cblockh_init);
   printf("  Mode switch (cblk_style): %d\n", P.mode);
   printf("  Irreversible (0: lossless): %d\n", P.irreversible);
   printf("  Region of interest (-1 means no ROI): %d\n", P.roi_compno);
   printf("  Region of interest, upshift value: %d\n", P.roi_shift);
   printf("  Number of precinct size specifications: %d\n", P.res_spec);
   if (P.res_spec > 0) {
      for (i = 0; i < P.res_spec; ++i)
         printf("    Precinct width, height:  %d  %d\n", P.prcw_init[i], P.prch_init[i]);
   }
   printf("\n  Command line encoder parameters (not used inside the library)\n");
   printf("    Input file name: %s\n", P.infile);
   printf("    Output file name: %s\n", P.outfile);
   printf("    DEPRECATED. Index generation: %d\n", P.index_on);
   printf("    DEPRECATED. Index string: %s\n", P.index);
   printf("    Subimage encoding: origin offset x, y: %d  %d\n", P.image_offset_x0, P.image_offset_y0);
   printf("    Subsampling value for dx, dy: %d  %d\n", P.subsampling_dx, P.subsampling_dy);
   printf("    Input file format 0: PGX, 1: PxM, 2: BMP 3:TIF : %d\n", P.decod_format);
   printf("    Output file format 0: J2K, 1: JP2, 2: JPT : %d\n", P.cod_format);

   printf("\n  UniPG>> NOT YET USED IN THE V2 VERSION OF OPENJPEG\n");
    ///**@name JPWL encoding parameters */
    ///*@{*/
    ///** enables writing of EPC in MH, thus activating JPWL */
    //OPJ_BOOL jpwl_epc_on;
    ///** error protection method for MH (0,1,16,32,37-128) */
    //int jpwl_hprot_MH;
    ///** tile number of header protection specification (>=0) */
    //int jpwl_hprot_TPH_tileno[JPWL_MAX_NO_TILESPECS];
    ///** error protection methods for TPHs (0,1,16,32,37-128) */
    //int jpwl_hprot_TPH[JPWL_MAX_NO_TILESPECS];
    ///** tile number of packet protection specification (>=0) */
    //int jpwl_pprot_tileno[JPWL_MAX_NO_PACKSPECS];
    ///** packet number of packet protection specification (>=0) */
    //int jpwl_pprot_packno[JPWL_MAX_NO_PACKSPECS];
    ///** error protection methods for packets (0,1,16,32,37-128) */
    //int jpwl_pprot[JPWL_MAX_NO_PACKSPECS];
    ///** enables writing of ESD, (0=no/1/2 bytes) */
    //int jpwl_sens_size;
    ///** sensitivity addressing size (0=auto/2/4 bytes) */
    //int jpwl_sens_addr;
    ///** sensitivity range (0-3) */
    //int jpwl_sens_range;
    ///** sensitivity method for MH (-1=no,0-7) */
    //int jpwl_sens_MH;
    ///** tile number of sensitivity specification (>=0) */
    //int jpwl_sens_TPH_tileno[JPWL_MAX_NO_TILESPECS];
    ///** sensitivity methods for TPHs (-1=no,0-7) */
    //int jpwl_sens_TPH[JPWL_MAX_NO_TILESPECS];
    ///*@}*/
    ///* <<UniPG */

   printf("\n  DEPRECATED Digital Cinema compliance 0-not , 1-yes : %d\n", P.cp_cinema);
   printf("  Maximum size (in bytes) for each component: %d\n", P.max_comp_size);
   printf("  DEPRECATED: RSIZ_CAPABILITIES: %d\n", P.cp_rsiz);
   printf("  Tile part generation: >%o<\n",(int)P.tp_on);
   printf("  Flag for Tile part generation: >%o<\n", (int)P.tp_flag);
   printf("  MCT (multiple component transform): >%o<\n", (int)P.tcp_mct);
   printf("  Enable JPIP indexing? : %d\n", P.jpip_on);
   printf("  Naive implementation of MCT: ");
   if (! P.mct_data) printf("NULL\n");
   else printf("%x\n", P.mct_data);
   printf("  Maximum size (in bytes) for the whole codestream: %d\n", P.max_cs_size);
   printf("  RSIZ value: %d\n", P.rsiz);

   printf("------------------------------------------------------------------------\n");
}

void print_image_comp_summary(opj_image_comp_t C)
{
   size_t i, num_pixels;
   printf("\n    IMAGE COMPONENT:\n");
   printf("    XRsiz, YRsiz: %u %u\n", C.dx, C.dy);
   printf("    Data width, height: %u %u\n", C.w, C.h);
   printf("    x, y component offset: %u %u\n", C.x0, C.y0);
   printf("    Precision: %u\n", C.prec);
   printf("    Image depth in bits: %u\n", C.bpp);
   printf("    Signed (1) / unsigned (0): %u\n", C.sgnd);
   printf("    Number of decoded resolution: %u\n", C.resno_decoded);
   printf("    Factor: %u\n", C.factor);
   printf("    Image component data:");
   num_pixels = C.w * C.h;
   for (i = 0; i < 102; ++i)
   {
      if (i % 17 == 0) printf("\n       ");
      printf("%4d", C.data[i]);
   }
   printf("\n");
   printf("    Alpha channel: %u\n", C.alpha);
}

void print_image_info (opj_image_t *I)
{
   size_t i;
   printf(" -- Image struct: ------------------------------------------------------------\n");
   printf("  XOsiz, YOsiz: %u %u\n", I->x0, I->y0);
   printf("  Xsiz, Ysiz (width, height): %u %u\n", I->x1, I->y1);
   printf("  Color space: sRGB, Greyscale or YUV: %d\n", I->color_space);
   printf("  Number of components in the image: %u\n", I->numcomps);
   /** image components */
   for (i = 0; i < I->numcomps; ++i)
      print_image_comp_summary(I->comps[i]);

   printf("  Size of ICC profile: %u\n",  I->icc_profile_len);
   printf("  'restricted' ICC profile:");
   if (I->icc_profile_len == 0) printf(" NULL\n\n");
   else {
      printf("\n   >");
      for (i = 0; i < I->icc_profile_len; ++i)
         printf("%c", I->icc_profile_buf[i]);
      printf("<\n\n");
   }
}

void print_image_stuff(opj_image_t *i, opj_cparameters_t p)
{
   printf("\nInput image: %s\n", p.infile);
   printf("Image data:\nx0:       %d\ny0:       %d\n",i->x0, i->y0);
   printf("x1:       %d\ny1:       %d\n",i->x1, i->y1);
   printf("numcomps: %d\nprof_len: %d\n",i->numcomps, i->icc_profile_len);
}

void get_header_values(char *tile_in,
                       OPJ_UINT32 *nc,
                       int *ih, int *iw)
{
   int *buffer = (int *) tile_in;

   *nc = buffer[0];       // number of color components
   *iw = buffer[1];       // image width
   *ih = buffer[2];       // image height

/** Do we need these? **
   *c_space = buffer[5];  // color space ASSUME set from num_comps
   *t_num = buffer[6];    // tile number ASSUME the number makes no difference
**/

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
    size_t len, compno;

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
    int irreversible = 0;
    int cblockw_init = 64;
    int cblockh_init = 64;
    int numresolution = 6;
    OPJ_UINT32 offsetx = 0;
    OPJ_UINT32 offsety = 0;
    int quality_loss = 0; // lossless
    
    int dim_max, tile_factor;

    get_header_values(tile_in, &num_comps, &image_height, &image_width);

// Use whole "image" as single tile; tiling set in TileDB level
     tile_factor = 1;
     dim_max = opj_uint_max(image_width, image_height);
     while ((dim_max / tile_factor) > 10000)
        tile_factor *= 2;

    tile_width = image_width / tile_factor;
    if (image_width % tile_factor) ++tile_width;
    tile_height = image_height / tile_factor;
    if (image_height % tile_factor) ++tile_height;

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
//  l_param.tile_size_on = OPJ_TRUE;
//  l_param.cp_tdx = tile_width;
//  l_param.cp_tdy = tile_height;

    /* Setting up for lossless **/
    l_param.cp_disto_alloc = 1;
    l_param.cp_fixed_alloc = 0;
    l_param.tcp_numlayers = 1;
    l_param.tcp_rates[0] = 0.0;
    l_param.tcp_distoratio[0] = 0.0;

    /* code block size */
    l_param.cblockw_init = cblockw_init;
    l_param.cblockh_init = cblockh_init;

    /* use irreversible encoding ?*/
    l_param.irreversible = irreversible;

    /** number of resolutions */
    l_param.numresolution = numresolution;

    /** Shouldn't need these, since there is no file input **/
    //l_param.decod_format = 14;
    l_param.cod_format = 1;

    /** progression order to use*/
    l_param.prog_order = OPJ_LRCP; // default

    /** Multiple component transform? **/
    if (num_comps == 1)
       l_param.tcp_mct = 0;
    else 
       l_param.tcp_mct = 1; /* multiple components in image */

    /* image definition */
    l_current_param_ptr = l_image_params;
    for (i = 0; i < num_comps; ++i) {
        l_current_param_ptr->bpp = 8;    // Does this need to be parameter?
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
       l_image = opj_image_create(num_comps, l_image_params, OPJ_CLRSPC_SRGB);
    else if (num_comps == 1)
       l_image = opj_image_create(num_comps, l_image_params, OPJ_CLRSPC_GRAY);
    else 
       l_image = opj_image_create(num_comps, l_image_params, OPJ_CLRSPC_UNKNOWN);
    if (! l_image) {
        //free(l_data);
        opj_destroy_codec(l_codec);
        return 1;
    }

// CPB: Are these needed?
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

// Copy pixels to l_image component data from l_data (tile_in)

   for (compno = 0; compno < num_comps; ++compno) {
      size_t num_bytes = image_width * image_height * sizeof(OPJ_INT32);
      memcpy(l_image->comps[compno].data, l_data, num_bytes);
      l_data += num_bytes;
   }

// CPB
//print_image_info(l_image);
//print_parameters(l_param);
// CPB end

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

    if (! opj_encode(l_codec, l_stream)) {
       fprintf(stderr, "ERROR -> opj_encode: failed to encode image\n");
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






