#include "util/geUtil.h"
#include "image/geImage.h"

#include "png_internal.h"
#include "png_external.h"

namespace GE
{

  ImageDecoderPNG::ImageDecoderPNG()
  {
    endings.pushBack("png");
  }
  
  /*===================================================
   *
   * File source manager
   *
   *===================================================*/

  /*
     This is a custom reading function that reads from
     a stdio FILE. The reason we are implementing this
     despite the default png_init_io() function is that
     the default one seems to be buggy and crashes on
     Windows. This function is to be passed into the
     png_set_read_fn() function with a FILE pointer
     along to register the custom reading callback:

     FILE *f; //assume it has been opened
     png_struct *ps; //assume it has been initialized
     png_set_read_fn(ps, f, pnguReadFileCallback);

   */

  void pnguReadFileCallback(png_struct *ps, png_byte *data, png_size_t length)
  {
    FILE *f = (FILE*)png_get_io_ptr(ps);
    png_size_t bytesread = fread(data, 1, length, f);
    if (bytesread != length) png_error(ps, "Read Error!");
  }

  /*===================================================
   *
   * Memory source manager
   *
   *===================================================*/

  /*
     Here we implement a structure to hold pointer to
     a memory block where encoded image data should
     reside along with data size and reading cursor.
     The reader should be 'opened' via the special
     pnguMemOpen() function, passing in the required
     arguments:

     int size;
     ubyte *data; // suppose image data is here
     PNGUMemReader r;
     pnguMemOpen(&r, data, size);

     To read data from memory the pnguMemRead() is
     called with the similar arguments as fread has:

     pnguMemRead(&r, dst_data, bytes_to_read);

     The function returns the number of bytes read
     which is smaller than the requested number in
     case the memory end has been reached.

     To use the memory source pass the address of the
     appropriate callback function to png_set_read_fn(),
     along with the address of the memory reader:

     png_struct *ps; //assume it has been initialized
     png_set_read_fn(ps, &r, pnguMemCallback);
  */

  typedef struct {

    size_t size;
    const BYTE *data;
    const BYTE *cursor;

  } PNGUMemReader;

  void pnguMemOpen(PNGUMemReader *r, const BYTE *data, size_t size)
  {
    r->size = size;
    r->data = data;
    r->cursor = data;
  }

  size_t pnguMemRead(PNGUMemReader *r, BYTE *dst, size_t n)
  {
    size_t bytes_till_end = r->data + r->size - r->cursor;
    if (n > bytes_till_end) n = bytes_till_end;
    memcpy(dst, r->cursor, n);
    r->cursor += n;
    return n;
  }

  void pnguReadMemCallback(png_struct *ps, png_byte *data, png_size_t length)
  {
    PNGUMemReader *r = (PNGUMemReader*)png_get_io_ptr(ps);
    png_size_t bytesread = pnguMemRead(r, data, length);
    if (bytesread != length) png_error(ps, "Read Error!");
  }


  /*===================================================
   *
   * High level PNG reading functions
   *
   *===================================================*/

  ImageErrorCode pnguInitStructures(png_struct **psp, png_info **pinfop, png_info **pendinfop,
                                    PNGUErrorPtr error_ptr,
                                    PNGUErrorFuncType error_fn,
                                    PNGUWarningFuncType warning_fn)
  {
    /* Create PNG read structure (default error handling is via setjmp) */
    *psp = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                  error_ptr, error_fn, warning_fn);
    if (!(*psp)) {
      return IMAGE_OUT_OF_MEMORY_ERROR; }

    /* Create PNG info structure */
    *pinfop = png_create_info_struct(*psp);
    if (!(*pinfop)) {
      png_destroy_read_struct(psp, NULL, NULL);
      return IMAGE_OUT_OF_MEMORY_ERROR; }

    /* Create PNG end-info structure */
    *pendinfop = png_create_info_struct(*psp);
    if (!(*pendinfop)) {
      png_destroy_read_struct(psp, pinfop, NULL);
      return IMAGE_OUT_OF_MEMORY_ERROR; }

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode pnguInitFileSource(png_struct *ps, FILE *infile)
  {
    BYTE psig[PNGU_SIG_BYTES];

    /* Catch errors here */
    if (pngu_jmp_error_caught(ps))
      return IMAGE_INVALID_DATA_ERROR;

     /* Try to find PNG signature to see if it's real PNG file */
    fread(psig, 1, PNGU_SIG_BYTES, infile);
    if (png_sig_cmp(psig, 0, 8) != 0) {
      fclose(infile);
      return IMAGE_NO_SIGNATURE_ERROR; }
    
    /* Initialize reading from file */
    png_set_read_fn(ps, infile, pnguReadFileCallback);
    
    /* Report bytes read for sig check */
    png_set_sig_bytes(ps, PNGU_SIG_BYTES);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode pnguInitMemSource(png_struct *ps, PNGUMemReader *reader)
  {
    BYTE psig[PNGU_SIG_BYTES];

    /* Catch errors here */
    if (pngu_jmp_error_caught(ps))
      return IMAGE_INVALID_DATA_ERROR;

    /* Try to find PNG signature to see if it's real PNG file */
    pnguMemRead(reader, psig, PNGU_SIG_BYTES);
    if (png_sig_cmp(psig, 0, 8) != 0) {
      return IMAGE_NO_SIGNATURE_ERROR; }

    /* Initialize reading from memory */
    png_set_read_fn(ps, reader, pnguReadMemCallback);
    
    /* Report bytes read for sig check */
    png_set_sig_bytes(ps, PNGU_SIG_BYTES);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode pnguReadInfo(Image *img, png_struct *ps, png_info *pinfo)
  {
    int png_width;
    int png_height;
    int png_color_type;
    int png_bit_depth;
    
    /* Catch errors here */
    if (pngu_jmp_error_caught(ps))
      return IMAGE_INVALID_DATA_ERROR;

    /* Read info first */
    png_read_info(ps, pinfo);
    png_width = png_get_image_width(ps, pinfo);
    png_height = png_get_image_height(ps, pinfo);
    png_color_type = png_get_color_type(ps, pinfo);
    png_bit_depth = png_get_bit_depth(ps, pinfo);

    /* Set transformations to normalize various PNG data
       formats to some more typical ones for easier handling */
    if (png_color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(ps);

    if (png_color_type == PNG_COLOR_TYPE_GRAY && png_bit_depth < 8)
      png_set_gray_1_2_4_to_8(ps);

    if (png_get_valid(ps, pinfo, PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(ps);

    if (png_bit_depth == 16)
      png_set_strip_16(ps);

    /* Set proper output color format */
    switch (png_color_type) {
    case PNG_COLOR_TYPE_GRAY:
      img->format = COLOR_FORMAT_GRAY;
      img->bpp = 1;
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      img->format = COLOR_FORMAT_GRAY_ALPHA;
      img->bpp = 2;
      break;
    case PNG_COLOR_TYPE_PALETTE:
      img->format = COLOR_FORMAT_RGB;
      img->bpp = 3;
      break;
    case PNG_COLOR_TYPE_RGB:
      img->format = COLOR_FORMAT_RGB;
      img->bpp = 3;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      img->format = COLOR_FORMAT_RGB_ALPHA;
      img->bpp = 4;
      break; }
    
    /* Other output image properties */
    img->width = png_get_image_width(ps, pinfo);
    img->height = png_get_image_height(ps, pinfo);
    img->stride = img->width * img->bpp;
    img->data = NULL;
    
    /* Allocate image data */
    img->data = (BYTE*)malloc(img->height * img->stride);
    if (!img->data) return IMAGE_OUT_OF_MEMORY_ERROR;
    
    /* Mark image info and color buffer are valid for use
       (though colors not read yet) */
    img->valid = true;

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode pnguReadScanlines(Image *img, png_struct *ps)
  {
    int r;
    png_bytep *rows = NULL;

    /* Allocate rows array */
    rows = (png_bytep*)malloc(img->height * sizeof(png_bytep));
    if (!rows) return IMAGE_OUT_OF_MEMORY_ERROR;

    /* Make rows point to appropriate memory */
    for (r=0; r<img->height; ++r)
      rows[r] = (png_bytep)(img->data + r * img->stride);
    
    /* Catch errors here */
    if (pngu_jmp_error_caught(ps))
      { free(rows); return IMAGE_INVALID_DATA_ERROR; }
    
    /* Read image */
    png_read_image(ps, rows);

    /* Cleanup */
    free(rows);

    return IMAGE_NO_ERROR;
  }

  ImageErrorCode ImageDecoderPNG::readFile(Image *img, const String &filename)
  {
    FILE *infile;
    png_struct *ps;
    png_info *pinfo;
    png_info *pendinfo;
    ImageErrorCode err;

    /* Invalidate image if not just type-checking */
    if (img != NULL) img->valid = false;
    
    /* Try to open file */
    infile = fopen(filename.toCSTR().buffer(), "rb");
    if (!infile) return IMAGE_FILE_READ_ERROR;

    /* Create PNG read and info structures */
    err = pnguInitStructures(&ps, &pinfo, &pendinfo, NULL, NULL, NULL);
    if (err != IMAGE_NO_ERROR) {
      fclose(infile);
      return err; }

    /* Setup reading from file */
    err = pnguInitFileSource(ps, infile);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      fclose(infile);
      return err; }

    /* Stop if just type-checking */
    if (img == NULL) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      fclose(infile);
      return IMAGE_NO_ERROR; }

    /* Read info and set image properties */
    err = pnguReadInfo(img, ps, pinfo);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      fclose(infile);
      return err; }

    /* Read image data */
    err = pnguReadScanlines(img, ps);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      fclose(infile);
      return err; }
    
    /* End reading (we could pass pendifo) instead of NULL
       if interested in data which is in the chunks after
       the image data (e.g. date, comments, ...) */
    png_read_end(ps, NULL);
    
    /* Cleanup */
    png_destroy_read_struct(&ps, &pinfo, &pendinfo);
    fclose(infile);
    return IMAGE_NO_ERROR;
  }

  ImageErrorCode ImageDecoderPNG::readData(Image *img, const BYTE *data, int size)
  {
    png_struct *ps;
    png_info *pinfo;
    png_info *pendinfo;
    PNGUMemReader reader;
    ImageErrorCode err;

    /* Invalidate image if not just type-checking */
    if (img != NULL) img->valid = 0;

    /* Init memory reader */
    pnguMemOpen(&reader, data, size);

    /* Create PNG read and info structures */
    err = pnguInitStructures(&ps, &pinfo, &pendinfo, NULL, NULL, NULL);
    if (err != IMAGE_NO_ERROR) {
      return err; }

    /* Setup reading from memory */
    err = pnguInitMemSource(ps, &reader);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      return err; }

    /* Stop if just type-checking */
    if (img == NULL) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      return IMAGE_NO_ERROR; }

    /* Read info and set image properties */
    err = pnguReadInfo(img, ps, pinfo);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      return err; }

    /* Read image data */
    err = pnguReadScanlines(img, ps);
    if (err != IMAGE_NO_ERROR) {
      png_destroy_read_struct(&ps, &pinfo, &pendinfo);
      return err; }
    
    /* End reading (we could pass pendifo) instead of NULL
       if interested in data which is in the chunks after
       the image data (e.g. date, comments, ...) */
    png_read_end(ps, NULL);
    
    /* Cleanup */
    png_destroy_read_struct(&ps, &pinfo, &pendinfo);
    return IMAGE_NO_ERROR;
  }
  
}/* namespace GE */
