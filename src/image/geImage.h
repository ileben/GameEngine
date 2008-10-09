#ifndef __GEIMAGE_H
#define __GEIMAGE_H

namespace GE
{
  class Image;
  class ImageDecoder;
  class ImageEncoder;
  class ImageDecoderJPEG;
  class ImageEncoderJPEG;
  class ImageDecoderPNG;
  
  enum ImageErrorCode
  {
    IMAGE_NO_ERROR                  = 0,
    IMAGE_FILE_READ_ERROR           = 2,
    IMAGE_FILE_WRITE_ERROR          = 3,
    IMAGE_NO_SIGNATURE_ERROR        = 4,
    IMAGE_INVALID_DATA_ERROR        = 5,
    IMAGE_UNSUPPORTED_TYPE_ERROR    = 6,
    IMAGE_INCOMPATIBLE_FORMAT_ERROR = 7,
    IMAGE_INVALID_IMAGE_ERROR       = 8,
    IMAGE_INVALID_ARGUMENT_ERROR    = 9,
    IMAGE_OUT_OF_MEMORY_ERROR       = 10
  };
  
  enum ColorFormat
  {
    COLOR_FORMAT_GRAY            = 0,
    COLOR_FORMAT_GRAY_ALPHA      = 1,
    COLOR_FORMAT_RGB             = 2,
    COLOR_FORMAT_RGB_ALPHA       = 3,
    COLOR_FORMAT_UNKNOWN         = 4
  };
    
  enum ScaleFilter
  {
    SCALE_FILTER_NEAREST     = 0,
    SCALE_FILTER_LINEAR      = 1  
  };
  
  class Color
  {
  public:
    float r;
    float g;
    float b;
    float a;
    
    Color() {r=0.0f; g=0.0f; b=0.0f; a=0.0f;}
    Color(float rr, float gg, float bb) {r=rr; g=gg; b=bb; a=1.0f;}
    Color(float rr, float gg, float bb, float aa) {r=rr; g=gg; b=bb; a=aa;}
    Color(const Color &c) {r=c.r; g=c.g; b=c.b; a=c.a;}
    Color& set(float rr, float gg, float bb, float aa=1.0f) {r=rr; g=gg; b=bb; a=aa; return *this;}
    Color& operator=(const Color &c) {r=c.r; g=c.g; b=c.b; a=c.a; return *this;}
  };
    
  struct ColorFormatDesc
  {
    ColorFormat format;
    int bpp;
    
    int rmask;
    int rshift;
    int rmax;
    
    int gmask;
    int gshift;
    int gmax;
    
    int bmask;
    int bshift;
    int bmax;
    
    int amask;
    int ashift;
    int amax;
  };

  class Image
  { 
  public: //TODO: gotta find a way to make this accessible to all coders
  
    Byte *data;
    int width;
    int height;
    int bpp;
    int stride;
    bool valid;
    ColorFormat format;
    ColorFormatDesc fd;
    
  private:
    
    static int ClassCount;
    static bool LittleEndian;
    static ArrayList<ImageDecoder*> *Decoders;
    static ArrayList<ImageEncoder*> *Encoders;
    
    static void prepareDescriptor(ColorFormatDesc *fd, ColorFormat f);
    static void storeColor(const Color &c, BYTE *pixel, const ColorFormatDesc &fd);
    static void loadColor(Color *c, BYTE *pixel, const ColorFormatDesc &fd);
    static void flipBytes(void *in, int size);
    
    static void copyPixels(BYTE *dst, ColorFormat dstFormat, int dstStride,
                          BYTE *src, ColorFormat srcFormat, int srcStride,
                          int dwidth, int dheight, int swidth, int sheight,
                          int dx, int dy, int sx, int sy,
                          int width, int height);
    
    ImageErrorCode read(bool usefile, const String &filename,
         const BYTE *data, int size, const String &typeHint);
         
    ImageErrorCode write(bool usefile, const String &filename,
         BYTE *data, int size, void *params, const String &type);

  public:
  
    Image();
    ~Image();
    
    bool isValid() const;
    int getWidth() const;
    int getHeight() const;
    BYTE* getData() const;
    ColorFormat getFormat() const;

    ImageErrorCode readFile(const String &filename, const String &typeHint);
    ImageErrorCode readData(const BYTE *data, int size, const String &typeHint);
    ImageErrorCode writeFile(const String &filename, void *params, const String &type);
    ImageErrorCode create(int width, int height, ColorFormat format, const Color &color);
    ImageErrorCode copy(Image *dst, ColorFormat newFormat);
    ImageErrorCode scale(Image *dst, int width, int height, ColorFormat format, ScaleFilter mode);
    ImageErrorCode setPixel(int x, int y, const Color &color);
    Color getPixel(int x, int y);
    ImageErrorCode drawLine(float x1, float y1, float x2, float y2, const Color &color);

    static char* FindFileType(const String &filename);
    static char* FindDataType(const BYTE *data, int size);
  };
  
  class ImageDecoder
  {
  protected:
    ArrayList<String> endings;
    
  public:
    bool isTypeSupported(const String &ending);
    virtual ImageErrorCode readFile(Image *img, const String &filename) = 0;
    virtual ImageErrorCode readData(Image *img, const BYTE *data, int size) = 0;
  };
  
  class ImageEncoder
  {
  protected:
    ArrayList<String> endings;
    
  public:
    bool isTypeSupported(const String &ending);
    virtual ImageErrorCode writeFile(Image *img, const String &filename, void *params) = 0;
  };
  
  struct EncoderParamsJPEG
  {
    int quality;
  };

  struct EncoderParamsPNG
  {
    int compressionLevel;
  };
  
}//namespace GE

#include "jpeg_external.h"
#include "png_external.h"

#endif//__GEIMAGE_H
