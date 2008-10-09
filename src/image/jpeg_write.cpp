#include "util/geUtil.h"
#include "image/geImage.h"

#include "jpeg_internal.h"
#include "jpeg_external.h"

namespace GE
{

ImageEncoderJPEG::ImageEncoderJPEG()
{
  endings.pushBack("jpg");
  endings.pushBack("jpeg");
}

/*=========================================================
 *
 * High level functions for writing
 *
 *=========================================================*/

  ImageErrorCode writeJpegCompressParams(Image *img, jpeg_compress_struct *jcp,
                                         EncoderParamsJPEG *params)
  {
    /* Setup image info */
    jcp->image_width = img->width;
    jcp->image_height = img->height;
    
    /* Setup color format */
    switch (img->format) {
    case COLOR_FORMAT_GRAY:
      jcp->input_components = 1;
      jcp->in_color_space = JCS_GRAYSCALE;
      break;
    case COLOR_FORMAT_RGB:
      jcp->input_components = 3;
      jcp->in_color_space = JCS_RGB;
      break;
    }

    /* Compression parameters */
    jpeg_set_defaults(jcp);
    jpeg_set_quality(jcp, params->quality, TRUE);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode writeJpegScanlines(Image *img, jpeg_compress_struct *jcp)
  {
    /* An array of rows (image array) holding a single row,
       since we are processing scanlines one-by-one */
    JSAMPROW buffer[1];

    /* Catch errors here */
    if (jpeg_jmp_error_caught(jcp))
      return IMAGE_FILE_WRITE_ERROR;

    /* Start compression */
    jpeg_start_compress(jcp, TRUE);

    /* Iterate until all scanlines processed */
    while (jcp->next_scanline < jcp->image_height) {

      /* Make row point to right image data and write a scanline */
      int y = jcp->next_scanline;
      buffer[0] = (JSAMPROW)(img->data + y * img->stride);
      jpeg_write_scanlines(jcp, buffer, 1);
    }

    /* Finalize compression */
    jpeg_finish_compress(jcp);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode ImageEncoderJPEG::writeFile(Image *img, const String &filename, void *params)
  {
    FILE *outfile = NULL;
    jpeg_compress_struct jcp;
    jpeg_jmp_error_mgr jerrmgr;
    ImageErrorCode err = IMAGE_NO_ERROR;
    EncoderParamsJPEG *jparams = (EncoderParamsJPEG*)params;

    /* Check if color format supported for compression*/
    if (img->format != COLOR_FORMAT_GRAY && img->format != COLOR_FORMAT_RGB)
      return IMAGE_INCOMPATIBLE_FORMAT_ERROR;

    /* Try to open file */
    outfile = fopen(filename.toCSTR().buffer(), "wb");
    if (outfile == NULL) return IMAGE_FILE_WRITE_ERROR;

    /* Init longjmp error manager */
    jcp.err = jpeg_jmp_error(&jerrmgr);

    /* Init compressor */
    jpeg_create_compress(&jcp);

    /* Set file destination */
    jpeg_stdio_dest(&jcp, outfile);
    
    /* Setup compression parameters */
    err = writeJpegCompressParams(img, &jcp, jparams);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_compress(&jcp);
      fclose(outfile);
      return err; }

    /* Write image data */
    err = writeJpegScanlines(img, &jcp);
    if (err != IMAGE_NO_ERROR) {
      jpeg_destroy_compress(&jcp);
      fclose(outfile);
      return err; }

    /* Cleanup */
    jpeg_destroy_compress(&jcp);
    fclose(outfile);
    return IMAGE_NO_ERROR;
  }

}/* namespace GE */
