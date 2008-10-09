#include "util/geUtil.h"
#include "image/geImage.h"

namespace GE
{

  /*=====================================================================
   *
   * Helper macros
   *
   *=====================================================================*/

  #define IMAGE_MAX(a,b) ( (a > b) ? a : b )
  #define IMAGE_MIN(a,b) ( (a < b) ? a : b )
  #define IMAGE_PIXEL(x,y, data, stride, bpp) (data + y*stride + x*bpp)

  /*=====================================================================
   *
   * Decoder
   *
   *=====================================================================*/
   
  bool ImageDecoder::isTypeSupported(const String &ending)
  {
    for (UintSize e=0; e<endings.size(); ++e)
      if (endings[e].equalsIgnoreCase (ending))
      return true;
    return false;
  }
   
  /*=====================================================================
   *
   * Encoder
   *
   *=====================================================================*/
   
  bool ImageEncoder::isTypeSupported(const String &ending)
  {
    for (UintSize e=0; e<endings.size(); ++e)
     if (endings[e].equalsIgnoreCase (ending))
      return true;
    return false;
  }

  /*=====================================================================
   *
   * Image
   *
   *=====================================================================*/
  
  
  /*----------------------------
   * Statics
   *----------------------------*/
   
  int Image::ClassCount = 0;
  bool Image::LittleEndian = false;
  ArrayList<ImageDecoder*> *Image::Decoders = NULL;
  ArrayList<ImageEncoder*> *Image::Encoders = NULL;

  /*--------------------------
   * Constructor / Destructor
   *--------------------------*/
   
  Image::Image()
  {
    //Initialize the static part of the class
    if (++Image::ClassCount == 1) {
      
      //Find endianness
      int testEndian = 1;
      LittleEndian = ( ((char*)&testEndian)[0] ==  1 );

      //Create decoder/encoder arrays
      Image::Decoders = new ArrayList<ImageDecoder*>;
      Image::Encoders = new ArrayList<ImageEncoder*>;
      
      //Load decoders/encoders
      Image::Decoders->pushBack(new ImageDecoderJPEG);
      Image::Encoders->pushBack(new ImageEncoderJPEG);
      Image::Decoders->pushBack(new ImageDecoderPNG);
    }
    
    valid = false;
    data = NULL;
    width = 0;
    height = 0;
    stride = 0;
    bpp = 0;
    format = COLOR_FORMAT_UNKNOWN;
  }

  Image::~Image()
  {
    //Destroy the static part of the class
    if (--Image::ClassCount == 0) {

      for (UintSize d=0; d<Image::Decoders->size(); ++d)
        delete Image::Decoders->elementAt(d);

      for (UintSize e=0; e<Image::Encoders->size(); ++e)
        delete Image::Encoders->elementAt(e);

      delete Image::Decoders;
      delete Image::Encoders;
    }
    
    //Free pixel data
    if (data != NULL) {
      free(data);
      data = NULL;
    }
  }
  
  bool Image::isValid() const {
    return valid;
  }
  
  int Image::getWidth() const {
    return width;
  }
  
  int Image::getHeight() const {
    return height;
  }
  
  BYTE* Image::getData() const {
    return data;
  }
  
  ColorFormat Image::getFormat() const {
    return format;
  }

  /*-----------------------------------------------------------
   * Reads image data from specified source (file or memory).
   * Type hint helps find the appropriate reader according to
   * the image file format. If empty string, the output of the
   * first reader that processes the data without error
   * will be used.
   *-----------------------------------------------------------*/

  ImageErrorCode Image::read(bool usefile, const String &filename,
                             const BYTE *srcdata, int size, const String &typeHint)
  {
    ImageErrorCode err;
    ImageDecoder *dec = NULL;

    //Find reader according to hint
    for (UintSize d=0; d<Image::Decoders->size(); ++d) {
    
      if (typeHint != "") {
        //Only try the reader that matches the type hint
        if (Image::Decoders->at(d)->isTypeSupported(typeHint))
          {dec = Image::Decoders->at(d); break;}
      }else{
        //Use first reader that returns success on type-check
        if (usefile) err = Image::Decoders->at(d)->readFile(NULL, filename);
        else err = Image::Decoders->at(d)->readData(NULL, data, size);
        if (err == IMAGE_NO_ERROR) {dec = Image::Decoders->at(d); break;}
      }
    }

    //If no decoder, return unsupported
    if (dec == NULL)
      return IMAGE_UNSUPPORTED_TYPE_ERROR;
    
    //Dispose old image data and read new one
    if (data != NULL) {free(data); data = NULL;}
    if (usefile) return dec->readFile(this, filename);
    else return dec->readData(this, srcdata, size);
  }

  /*--------------------------------------------------------------
   * Writes image data to specified destination (file or memory).
   * Additional encoding parameters must be specified along with
   * the destination encoding, based on which the appropriate
   * writer is selected.
   *--------------------------------------------------------------*/

  ImageErrorCode Image::write(bool usefile, const String &filename,
                              BYTE *dstdata, int size, void *params, const String &type)
  {
    ImageEncoder *enc = NULL;

    for( UintSize e=0; e<Image::Encoders->size(); ++e ){
      //Check if writer hame matches the type
      if( Image::Encoders->at(e)->isTypeSupported(type) )
        {enc = Image::Encoders->at(e); break;}
    }

    //If no encoder, return unsupported
    if (enc == NULL)
      return IMAGE_UNSUPPORTED_TYPE_ERROR;

    //Write image data with selected encoder
    if (usefile) return enc->writeFile(this, filename, params);
    else return IMAGE_NO_ERROR; //Encoding to memory not implemented yet
  }

  /*-------------------------------------------------------
   * Reads image from file with given filename. Type hint
   * helps find the appropriate read according to the
   * image file format. If empty string, the output of the
   * first reader that processes the data without error
   * will be used.
   *-------------------------------------------------------*/

  ImageErrorCode Image::readFile(const String &filename, const String &typeHint)
  {
    return read (true, filename, NULL, 0, typeHint);
  }

  /*-------------------------------------------------------
   * Reads image from in-memory image data. Type hint
   * helps find the appropriate read according to the
   * image file format. If empty string, the output of the
   * first reader that processes the data without error
   * will be used.
   *-------------------------------------------------------*/

  ImageErrorCode Image::readData(const BYTE *data, int size, const String &typeHint)
  {
    return read (false, "", data, size, typeHint);
  }

  /*----------------------------------------------------
   * Writes image data to given file encoded according
   * to given type and encoding parameters
   *---------------------------------------------------*/
   
  ImageErrorCode Image::writeFile(const String &filename, void *params, const String &type)
  {
    return write (true, filename, NULL, 0, params, type);
  }

  /*-------------------------------------------------
   * Fills the format descriptor structure according
   * to the given color format.
   *-------------------------------------------------*/

  void Image::prepareDescriptor(ColorFormatDesc *fd, ColorFormat f)
  {
    //Mask lookup tables
    int rmask[] = {
      0xFF,        /* G */
      0xFF00,      /* GA */
      0xFF0000,    /* RGB */
      0xFF000000,  /* RGBA */
      0x0 };       /* UNKNOWN */
    
    int gmask[] = {
      0xFF,        /* G */
      0xFF00,      /* GA */
      0x00FF00,    /* RGB */
      0x00FF0000,  /* RGBA */
      0x0 };       /* UNKNOWN */
    
    int bmask[] = {
      0xFF,        /* G */
      0xFF00,      /* GA */
      0x0000FF,    /* RGB */
      0x0000FF00,  /* RGBA */
      0x0 };       /* UNKNOWN */
    
    int amask[] = {
      0x0,         /* G */
      0x00FF,      /* GA */
      0x000000,    /* RGB */
      0x000000FF,  /* RGBA */
      0x0 };       /* UNKNOWN */
    
    //The rest in the same fashion...
    int rshift[] = {0,  8, 16, 24, 0};
    int gshift[] = {0,  8,  8, 16, 0};
    int bshift[] = {0,  8,  0,  8, 0};
    int ashift[] = {0,  0,  0,  0, 0};
    
    int rmax[] = {255, 255, 255, 255, 255};
    int gmax[] = {255, 255, 255, 255, 255};
    int bmax[] = {255, 255, 255, 255, 255};
    int amax[] = {255, 255, 255, 255, 255};
    
    int bpp[]    = {8, 16, 24, 32, 0};
    
    //ColorFormat is index for lookup
    fd->format = f;
    fd->rmask = rmask[f];
    fd->gmask = gmask[f];
    fd->bmask = bmask[f];
    fd->amask = amask[f];
    fd->rshift = rshift[f];
    fd->gshift = gshift[f];
    fd->bshift = bshift[f];
    fd->ashift = ashift[f];
    fd->rmax = rmax[f];
    fd->gmax = gmax[f];
    fd->bmax = bmax[f];
    fd->amax = amax[f];
    fd->bpp = bpp[f];
  }

  /*-----------------------------------------------
   * Flips [size] number of bytes at given address
   *-----------------------------------------------*/

  void Image::flipBytes(void *in, int size)
  {
    BYTE *a = (BYTE*)in;
    int b; BYTE temp;
    int hsize = (size + (size & 0x1)) / 2;
    for (b=0; b<hsize; ++b) {
      temp = a[b];
      a[b] = a[size-b-1];
      a[size-b-1] = temp;
    }
  }

  /*-----------------------------------------------
   * Loads a color from pixel memory in the format
   * described by given format descriptor at the
   * specified address.
   *-----------------------------------------------*/

  void Image::loadColor(Color *c, BYTE *pixel, const ColorFormatDesc &fd)
  {
    unsigned int px = 0;
    int bytes = fd.bpp / 8;
    memcpy(&px, pixel, bytes);
    if (LittleEndian) flipBytes(&px, sizeof(int));
    px = px >> (sizeof(unsigned int)*8 - fd.bpp);
    c->r = (float) ((px & fd.rmask) >> fd.rshift) / fd.rmax;
    c->g = (float) ((px & fd.gmask) >> fd.gshift) / fd.gmax;
    c->b = (float) ((px & fd.bmask) >> fd.bshift) / fd.bmax;
    c->a = (float) ((px & fd.amask) >> fd.ashift) / fd.amax;
    if (fd.amask == 0x0) c->a = 1.0;
  }

  /*-----------------------------------------------
   * Stores a color to pixel memory in the format
   * described by given format descriptor at the
   * specified address.
   *-----------------------------------------------*/

  void Image::storeColor(const Color &c, BYTE *pixel, const ColorFormatDesc &fd)
  {
    unsigned int px = 0;
    int bytes = fd.bpp / 8;
    px = ( (( (int)(c.r * fd.rmax) << fd.rshift) & fd.rmask) |
           (( (int)(c.g * fd.gmax) << fd.gshift) & fd.gmask) |
           (( (int)(c.b * fd.bmax) << fd.bshift) & fd.bmask) |
           (( (int)(c.a * fd.amax) << fd.ashift) & fd.amask) );
    px = px << (sizeof(unsigned int)*8 - fd.bpp);
    if (LittleEndian) flipBytes(&px, sizeof(int));
    memcpy(pixel, &px, bytes);
  }

  /*-------------------------------------------
   * Sets a single pixel in the current image
   *-------------------------------------------*/

  ImageErrorCode Image::setPixel(int x, int y, const Color &color)
  {
    ColorFormatDesc fd;
    BYTE *px;

    if (data == NULL)
      return IMAGE_INVALID_IMAGE_ERROR;

    if (x < 0 || x >= width ||
        y < 0 || y >= height)
      return IMAGE_INVALID_ARGUMENT_ERROR;

    prepareDescriptor(&fd, format);
    px = IMAGE_PIXEL(x, y, data, stride, bpp);
    storeColor(color, px, fd);

    return IMAGE_NO_ERROR;
  }

  /*----------------------------------------------------------
   * Returns the color of a single pixel in the current image
   *----------------------------------------------------------*/

  Color Image::getPixel(int x, int y)
  {
    ColorFormatDesc fd;
    Color color;
    BYTE *px;
    
    if (x < 0 || x >= width ||
        y < 0 || y >= height)
      return color;

    prepareDescriptor(&fd, format);
    px = IMAGE_PIXEL(x, y, data, stride, bpp);
    loadColor(&color, px, fd);

    return color;
  }

  /*-----------------------------------------------------------------
   * Draw a line between given points using the simple DDA algorithm
   *-----------------------------------------------------------------*/

  ImageErrorCode Image::drawLine(float x1, float y1, float x2, float y2, const Color &color)
  {
    ColorFormatDesc fd;
    BYTE *pixel;
    float dx, dy, adx, ady;
    int xstep, xend, ix1, ix2, ix;
    float ystep, y;
    int swap = 0;

    if (data == NULL)
      return IMAGE_INVALID_IMAGE_ERROR;

    prepareDescriptor(&fd, format);

    /* Find signed and absolute difference */
    dx = x2 - x1;
    dy = y2 - y1;
    adx = (dx < 0.0f ? -dx : dx);
    ady = (dy < 0.0f ? -dy : dy);

    /* Swap coordinates if slope greater than 1 */
    if (ady > adx) {
      /* Swap coordinate 1 */
      float temp = x1;
      x1 = y1;
      y1 = temp;
      /* Swap coordinate 2 */
      temp = x2;
      x2 = y2;
      y2 = temp;
      /* Swap slopes */
      temp = adx;
      adx = ady;
      ady = temp;
      swap = 1;
    }

    /* Round first and last coord to int */
    ix1 = (int)(x1+0.5f);
    ix2 = (int)(x2+0.5f);

    /* Find X and Y step according to slope */
    if (adx == 0.0f) {
      /* Since the coords were swapped we
         are sure ady is 0.0f as well */
      ystep = 0.0f;
      xstep = 1;
      xend = ix2+1;
    }else{
      ystep = (y1 <= y2 ? ady/adx : -ady/adx);
      xstep = (ix1 <= ix2 ? 1 : -1);
      xend = (ix1 <= ix2 ? ix2+1 : ix2-1); 
    }

    /* Walk the slope */
    for (ix=ix1, y=y1; ix!=xend; ix+=xstep, y+=ystep) {

      int px = ix;
      int py = (int)(y+0.5f);

      if (swap) {
        int temp = px;
        px = py;
        py = temp;
      }
      
      if (px < 0 || px >= width) continue;
      if (py < 0 || py >= height) continue;

      pixel = IMAGE_PIXEL(px, py, data, stride, bpp);
      storeColor(color, pixel, fd);
    }

    return IMAGE_NO_ERROR;
  }

  /*----------------------------------------------------
   * Allocates image data for the given size and format
   * and initializes it to the specified color
   *----------------------------------------------------*/

  ImageErrorCode Image::create(int newWidth, int newHeight, ColorFormat newFormat, const Color &color)
  {
    ColorFormatDesc fd;
    BYTE *newData, *px;
    int newBpp;
    int newStride;
    int newSize;
    int x, y;

    if (newWidth<=0 || newHeight<=0)
      return IMAGE_INVALID_ARGUMENT_ERROR;

    /* Creat color format descriptor */
    prepareDescriptor(&fd, newFormat);

    /* Find bytes-per-pixel */
    switch (newFormat) {
    case COLOR_FORMAT_GRAY:       newBpp = 1; break;
    case COLOR_FORMAT_GRAY_ALPHA: newBpp = 2; break;
    case COLOR_FORMAT_RGB:        newBpp = 3; break;
    case COLOR_FORMAT_RGB_ALPHA:  newBpp = 4; break;
    default: return IMAGE_INVALID_ARGUMENT_ERROR;
    }

    /* Allocate new image data */
    newStride = newBpp * newWidth;
    newSize = newStride * newHeight;
    newData = (BYTE*)malloc(newSize);
    if (newData == NULL)
      return IMAGE_OUT_OF_MEMORY_ERROR;

    /* Initialize color */
    for (y=0; y<newHeight; ++y) {
      for (x=0; x<newWidth; ++x) {
        px = IMAGE_PIXEL(x, y, newData, newStride, newBpp);
        storeColor(color, px, fd);
      }}

    /* Free old image data */
    if (data != NULL)
      free(data);

    /* Assign new values */
    width = newWidth;
    height = newHeight;
    format = newFormat;
    bpp = newBpp;
    stride = newStride;
    data = newData;
    valid = true;

    return IMAGE_NO_ERROR;
  }

  /*------------------------------------------------------
   * Generic function for copying a rectangular block of
   * pixels of size (width,height) from source pixel block
   * of size (swidth, sheight) at coordinate (sx,sy) to
   * destination pixel block of size (dwidth, dheight) at
   * coordinate (dx, dy).
   *-------------------------------------------------------*/

  /*
    In order to optimize the copying loop and remove the
    if statements from it to check whether target pixel
    is in the source and destination surface, we clamp
    copy rectangle in advance. This is quite a tedious
    task though. Here is a picture of the scene. Note that
    (dx,dy) is actually an offset of the copy rectangle
    (clamped to src surface) from the (0,0) point on dst
    surface. A negative (dx,dy) (as in this picture) also
    affects src coords of the copy rectangle which have
    to be readjusted again (sx,sy,width,height).

                          src
    *----------------------*
    | (sx,sy)  copy rect   |
    | *-----------*        |
    | |\(dx, dy)  |        |          dst
    | | *------------------------------*
    | | |xxxxxxxxx|        |           |
    | | |xxxxxxxxx|        |           |
    | *-----------*        |           |
    |   |   (width,height) |           |
    *----------------------*           |
        |           (swidth,sheight)   |
        *------------------------------*
                                (dwidth,dheight)
  */

  void Image::copyPixels(BYTE *dst, ColorFormat dstFormat, int dstStride,
                         BYTE *src, ColorFormat srcFormat, int srcStride,
                         int dwidth, int dheight, int swidth, int sheight,
                         int dx, int dy, int sx, int sy,
                         int width, int height)
  {
    ColorFormatDesc dfd, sfd;
    int dxold, dyold;
    int SX, SY, DX, DY;
    int sBpp, dBpp;
    BYTE *SD, *DD;
    Color c;
    
    prepareDescriptor(&sfd, srcFormat);
    prepareDescriptor(&dfd, dstFormat);

    /* Cancel if copy rect out of src bounds */
    if (sx >= swidth || sy >= sheight) return;
    if (sx + width < 0 || sy + height < 0) return;
    
    /* Clamp copy rectangle to src bounds */
    sx = IMAGE_MAX(sx, 0);
    sy = IMAGE_MAX(sy, 0);
    width = IMAGE_MIN(width, swidth - sx);
    height = IMAGE_MIN(height, sheight - sy);
    
    /* Cancel if copy rect out of dst bounds */
    if (dx >= dwidth || dy >= dheight) return;
    if (dx + width < 0 || dy + height < 0) return;
    
    /* Clamp copy rectangle to dst bounds */
    dxold = dx; dyold = dy;
    dx = IMAGE_MAX(dx, 0);
    dy = IMAGE_MAX(dy, 0);
    sx += dx - dxold;
    sy += dy - dyold;
    width -= dx - dxold;
    height -= dy - dyold;
    width = IMAGE_MIN(width, dwidth  - dx);
    height = IMAGE_MIN(height, dheight - dy);
    
    /* Calculate stride from format if not given */
    sBpp = sfd.bpp / 8; dBpp = dfd.bpp / 8;
    if (srcStride == -1) srcStride = swidth * sBpp;
    if (dstStride == -1) dstStride = dwidth * dBpp;
    
    /* Walk pixels and copy */
    for (SY=sy, DY=dy; SY < sy+height; ++SY, ++DY) {
      SD = src + SY * srcStride + sx * sfd.bpp/8;
      DD = dst + DY * dstStride + dx * dfd.bpp/8;
      
      for (SX=sx, DX=dx; SX < sx+width; ++SX, ++DX) {
        loadColor(&c, SD, sfd);
        storeColor(c, DD, dfd);
        SD += sBpp; DD += dBpp;
      }}
  }

  /*-------------------------------------------------------
   * Scale [src] image to given size and stores output
   * into [dst] image in the given color format. It is
   * valid for [src] and [dst] to point to the same image.
   * Scale [mode] specifies how the scaled pixel colors
   * will be interpolated from source pixels.
   *-------------------------------------------------------*/

  ImageErrorCode Image::scale(Image *dst, int newWidth, int newHeight,
                              ColorFormat newFormat, ScaleFilter filter)
  {
    BYTE *newData;
    int newBpp;
    int newStride;
    int newSize;
    int x, y;
    ColorFormatDesc dfd, sfd;
    Color c00, c10, c11, c01, cout;
    float *cp00, *cp10, *cp11, *cp01, *cpout;
    
    if (data == NULL)
      return IMAGE_INVALID_IMAGE_ERROR;
    
    if (dst == this && format == newFormat &&
        width == newWidth && height == newHeight)
      return IMAGE_NO_ERROR;
    
    /* Obtain format descriptor */
    prepareDescriptor(&sfd, format);
    prepareDescriptor(&dfd, newFormat);
    newBpp = dfd.bpp / 8;
    newStride = newWidth * newBpp;
    newSize = newHeight * newStride;
    
    /* Allocate new data */
    newData = (BYTE*)malloc(newSize);
    if (newData == NULL) return IMAGE_OUT_OF_MEMORY_ERROR;
    
    /* Convert colors to float arrays */
    cp00 = (float*)&c00;
    cp10 = (float*)&c10;
    cp11 = (float*)&c11;
    cp01 = (float*)&c01;
    cpout = (float*)&cout;
    
    /* Scale pixels */
    for (y=0; y<newHeight; ++y) {
      for (x=0; x<newWidth; ++x) {

        float xout = x + 0.5f;
        float yout = y + 0.5f;
        float xin = (xout / newWidth) * width;
        float yin = (yout / newHeight) * height;
        
        if (filter == SCALE_FILTER_NEAREST) {
          
          int xval = (int)(xin);
          int yval = (int)(yin);
          BYTE *pixout = IMAGE_PIXEL(x, y, newData, newStride, newBpp);
          BYTE *pixin = IMAGE_PIXEL(xval, yval, data, stride, bpp);
          loadColor(&cout, pixin, sfd);
          storeColor(cout, pixout, dfd);
          

        }else if (filter == SCALE_FILTER_LINEAR) {
    
          int c;
          float x0, x1, y0, y1;
          float dL, dR, dT, dB;
          BYTE *p00, *p10, *p11, *p01, *pout;
          
          /* Take care of border pixels */
          if (xin <= 0.5f) xin += 1.0f;
          if (yin <= 0.5f) yin += 1.0f;
          if (xin >= (float)width-0.5f) xin -= 1.0f;
          if (yin >= (float)height-0.5f) yin -= 1.0f;
          /* Round to four nearest input pixels */
          x0 = (float)(int)(xin - 0.5f);
          x1 = (float)(int)(xin + 0.5f);
          y0 = (float)(int)(yin - 0.5f);
          y1 = (float)(int)(yin + 0.5f);
          /* Pick pixel addresses */
          p00 =  IMAGE_PIXEL((int)x0,(int)y0, data, stride, bpp);
          p10 =  IMAGE_PIXEL((int)x1,(int)y0, data, stride, bpp);
          p11 =  IMAGE_PIXEL((int)x1,(int)y1, data, stride, bpp);
          p01 =  IMAGE_PIXEL((int)x0,(int)y1, data, stride, bpp);
          pout = IMAGE_PIXEL(x,y, newData, newStride, newBpp);
          /* Get pixel colors */
          loadColor(&c00, p00, sfd);
          loadColor(&c10, p10, sfd);
          loadColor(&c11, p11, sfd);
          loadColor(&c01, p01, sfd);
          /* Offset back to pixel centers */
          x0 += 0.5f; x1 += 0.5f;
          y0 += 0.5f; y1 += 0.5f;
          /* Interpolate each color component */
          dL = xin - x0; dR = x1 - xin;
          dT = yin - y0; dB = y1 - yin;
          for (c=0; c<4; ++c) {
            float d1 = dR * cp00[c] + dL * cp10[c];
            float d2 = dR * cp01[c] + dL * cp11[c];
            cpout[c] = (dB * d1 + dT * d2); }
          /* Store interpolated pixel */
          storeColor(cout, pout, dfd);
        }
      }
    }
    
    /* Set new properties */
    if (dst->data != NULL)
      free(dst->data);
    dst->data = newData;
    dst->width = newWidth;
    dst->height = newHeight;
    dst->format = newFormat;
    dst->stride = newStride;
    dst->bpp = newBpp;
    
    return IMAGE_NO_ERROR;
  }

  /*------------------------------------------------------
   * Converts the [src] image data into given format and
   * stores the result into [dst] image. It is valid for
   * [src] and [dst] to point to the same image.
   *------------------------------------------------------*/

  ImageErrorCode Image::copy(Image *dst, ColorFormat newFormat)
  {
    ColorFormatDesc fd;
    BYTE *newData;
    int newBpp;
    int newStride;
    int newSize;
    
    if (data == NULL)
      return IMAGE_INVALID_IMAGE_ERROR;
    
    if (dst == this && format == newFormat)
      return IMAGE_NO_ERROR;
    
    /* Obtain format descriptor */
    prepareDescriptor(&fd, newFormat);
    newBpp = fd.bpp / 8;
    newStride = width * newBpp;
    newSize = height * newStride;
    
    /* Allocate new data */
    newData = (BYTE*)malloc(newSize);
    if (newData == NULL) return IMAGE_OUT_OF_MEMORY_ERROR;
    
    /* Convert pixels */
    copyPixels(newData, newFormat, newStride,
               data, format, stride,
               width, height, width, height,
               0, 0, 0, 0, width, height);
    
    /* Set new properties */
    if (dst->data != NULL)
      free(dst->data);
    dst->data = newData;
    dst->width = width;
    dst->height = height;
    dst->stride = newStride;
    dst->format = newFormat;
    dst->bpp = newBpp;
    
    return IMAGE_NO_ERROR;
  }
  
}/* namespace GE */
