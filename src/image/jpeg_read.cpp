#include "util/geUtil.h"
#include "image/geImage.h"

#include "jpeg_internal.h"
#include "jpeg_external.h"

namespace GE
{

  ImageDecoderJPEG::ImageDecoderJPEG()
  {
    endings.pushBack("jpg");
    endings.pushBack("jpeg");
  }
  
  /*=======================================================
   *
   * Custom in-memory data source
   *
   *=======================================================*/

  /*
     This is a custom data source to decompress JPEG image
     data stored in application memory rather than from a
     stdio stream. Since all the data is already in the
     buffer, rebuffering is actualy not needed and this
     manager is quite simple.

     To use the memory source manager just call the custom
     source initialization function instead of the default
     jpeg_stdio_src(...) one. It takes the pointer to the
     data and the size of the image data:

     struct jpeg_decompress_struct jdc;
     jpeg_mem_src(&jdc, my_image_data, my_image_bytesize);

   */

  /*----------------------------------------------
   * Memory source manager structure
   *----------------------------------------------*/

  struct jpeg_mem_source_mgr {
    /* Include 'base class' members */
    struct jpeg_source_mgr base;
    /* Pointer to in-memory jpeg data */
    JOCTET *data;
  };

  /*-------------------------------------------------
   * Structure and pointer typedefs
   *-------------------------------------------------*/

  typedef   struct jpeg_mem_source_mgr    jpeg_mem_source_mgr;
  typedef   jpeg_mem_source_mgr*          jpeg_mem_source_mgr_ptr;

  /*----------------------------------------------
   * Will be called by jpeg_read_header before
   * actually reading any data
   *----------------------------------------------*/

  void mem_source_init(j_decompress_ptr jdc)
  {
    /* Nothing to do */
  }

  /*---------------------------------------------------------------
   * Will be called when decompressor state requires more data
   * to be read. With all data in memory (and since we initialize
   * the amount of data in buffer directly to the actual size of
   * the input data) this should only happen in case the data was
   * erroneus and decompressor didn't meet the end of the source
   * before reaching the end of data.
   *---------------------------------------------------------------*/

  boolean mem_source_fill_buffer(j_decompress_ptr jdc)
  {
    /* Invoke exit error handler */
    ERREXIT(jdc, JERR_INPUT_EMPTY);
    return TRUE;
  }

  /*----------------------------------------------
   * Some uninteresting data is being skipped
   *----------------------------------------------*/

  void mem_source_skip_data(j_decompress_ptr jdc, long num_bytes)
  {
    /* We know it's memory source manager */
    jpeg_mem_source_mgr_ptr srcmgr =
      (jpeg_mem_source_mgr_ptr)jdc->src;
    /* Check if we are skipping beyond available data */
    if (srcmgr->base.bytes_in_buffer - (size_t)num_bytes < 0) {
      /* We should re-fill buffer. This means data was erroneus */
      ERREXIT(jdc, JERR_INPUT_EMPTY); }
    /* Just jump over */
    srcmgr->base.next_input_byte += (size_t) num_bytes;
    srcmgr->base.bytes_in_buffer -= (size_t) num_bytes;
  }

  /*----------------------------------------------
   * Terminating source
   *----------------------------------------------*/

  void mem_source_terminate (j_decompress_ptr cinfo)
  {
    /* Nothing to do */
  }

  /*--------------------------------------------------------
   * Prepares JPEG structure for input from in-memory data
   *--------------------------------------------------------*/

  void jpeg_mem_src(j_decompress_ptr jdc, const JOCTET *data, long size)
  {
    /* We just follow the example in jdatasrc.c and make the memory
       'permanent'. This supposedly optimizes reusing of the same
       decompressor structure for more than one image. Probably
       this means that source manager won't be deallocated upon
       calling jpeg_finish_decompress, but instead only when
       jpeg_destroy is called upon the decompressor structure. */
    jpeg_mem_source_mgr_ptr srcmgr;

    /* First time for this JPEG object? */
    if (jdc->src == NULL) {
      /* Allocate source manager structure */
      jdc->src = (jpeg_source_mgr*) (*jdc->mem->alloc_small)
        ((j_common_ptr)jdc, JPOOL_PERMANENT, sizeof(jpeg_mem_source_mgr));
    }

    /* Initialize base part of class */
    srcmgr = (jpeg_mem_source_mgr_ptr)jdc->src;
    srcmgr->base.init_source = mem_source_init;
    srcmgr->base.fill_input_buffer = mem_source_fill_buffer;
    srcmgr->base.skip_input_data = mem_source_skip_data;
    srcmgr->base.resync_to_restart = jpeg_resync_to_restart; /* Use default */
    srcmgr->base.term_source = mem_source_terminate;
    srcmgr->base.bytes_in_buffer = size;
    srcmgr->base.next_input_byte = data;
  }


  /*=====================================================
   *
   * High-level JPEG loading function examples
   *
   *=====================================================*/

  /*
     There are 2 functions provided: one uses the stdio
     source manager to read JPEG data from file, the other
     one uses the custom memory source manager to read JPEG
     data from an in-memory buffer.

     Both functions use LongJump error manager for maximum
     C compatibility. This is still not a problem since
     there are no C++ objects used that would need to be
     deconstructed when the long jump unwinds the stack.

   */

  ImageErrorCode readJpegInfo(Image *img, jpeg_decompress_struct* jdc)
  {
    /* Catch errors here */
    if (jpeg_jmp_error_caught(jdc))
      return IMAGE_INVALID_DATA_ERROR;

    /* Read header and start */
    jpeg_read_header(jdc, TRUE);
    jpeg_start_decompress(jdc);

    /* Stop if just type-checking */
    if (img == NULL) return IMAGE_NO_ERROR;
    
    /* Set proper output color format */
    switch (jdc->output_components) {
    case 1:
      img->format = COLOR_FORMAT_GRAY;
      img->bpp = 1;
      break;
    case 3: default:
      img->format = COLOR_FORMAT_RGB;
      img->bpp = 3;
      break;
    }

    /* Other image properties */
    img->width = jdc->output_width;
    img->height = jdc->output_height;
    img->stride = img->width * img->bpp;
    img->data = NULL;
    
    /* Allocate image buffer */
    img->data = (BYTE*)malloc(img->stride * img->height);
    if (!img->data) return IMAGE_OUT_OF_MEMORY_ERROR;

    /* Mark image info and color buffer are valid for use
       (though colors not read yet) */
    img->valid = true;

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode readJpegScanlines(Image *img, jpeg_decompress_struct* jdc)
  {
    /* An array of rows (image array) holding a single row,
       since we are processing scanlines one-by-one */
    JSAMPROW buffer[1];
    
    /* Catch errors here */
    if (jpeg_jmp_error_caught(jdc))
      return IMAGE_INVALID_DATA_ERROR;
    
    /* Iterate until all scanlines processed */
    while (jdc->output_scanline < (unsigned int)img->height) {
      
      /* Make row point to right image data and read a scanline */
      int y = jdc->output_scanline;
      buffer[0] = (JSAMPROW)(img->data + y * img->stride);
      jpeg_read_scanlines(jdc, buffer, 1);
    }

    /* Finalize decompression */
    jpeg_finish_decompress(jdc);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode ImageDecoderJPEG::readFile(Image *img, const String &filename)
  { 
    FILE *infile;
    jpeg_decompress_struct jdc;
    jpeg_jmp_error_mgr jerrmgr;
    ImageErrorCode err;

    /* Invalidate image if not just type-checking */
    if (img != NULL) img->valid = false;
    
    /* Try to open file */
    infile = fopen(filename.toCSTR().buffer(), "rb");
    if (infile == NULL) return IMAGE_FILE_READ_ERROR;
    
    /* Init longjmp error manager */
    jdc.err = jpeg_jmp_error(&jerrmgr);
    
    /* Init decompressor */
    jpeg_create_decompress(&jdc);
    
    /* Set file source */
    jpeg_stdio_src(&jdc, infile);

    /* Read jpeg info */
    err = readJpegInfo(img, &jdc);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_decompress(&jdc);
      fclose(infile);
      return err; }

    /* Stop if just type-checking */
    if (img == NULL) {
      jpeg_destroy_decompress(&jdc);
      fclose(infile);
      return IMAGE_NO_ERROR; }
    
    /* Read jpeg data */
    err = readJpegScanlines(img, &jdc);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_decompress(&jdc);
      fclose(infile);
      return err; }
    
    /* Cleanup */
    jpeg_destroy_decompress(&jdc);
    fclose(infile);
    return IMAGE_NO_ERROR;
  }

  ImageErrorCode ImageDecoderJPEG::readData(Image *img, const BYTE *data, int size)
  {
    jpeg_decompress_struct jdc;
    jpeg_jmp_error_mgr jerrmgr;
    ImageErrorCode err;

    /* Invalidate image if not just type-checking */
    if (img != NULL) img->valid = false;
    
    /* Init longjmp error manager */
    jdc.err = jpeg_jmp_error(&jerrmgr);
    
    /* Init decompressor */
    jpeg_create_decompress(&jdc);

    /* Set memory source */
    jpeg_mem_src(&jdc, data, size);

    /* Read jpeg info */
    err = readJpegInfo(img, &jdc);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_decompress(&jdc);
      return err; }

    /* Stop if just type-checking */
    if (img == NULL) {
      jpeg_destroy_decompress(&jdc);
      return IMAGE_NO_ERROR; }

    /* Read jpeg data */
    err = readJpegScanlines(img, &jdc);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_decompress(&jdc);
      return err; }
    
    /* Cleanup */
    jpeg_destroy_decompress(&jdc);
    return IMAGE_NO_ERROR;
  }
  
}/* namespace GE */
