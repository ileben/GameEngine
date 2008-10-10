#ifndef __USEJPEG_H
#define __USEJPEG_H

namespace GE
{

  class ImageDecoderJPEG : public ImageDecoder
  {
  public:
    ImageDecoderJPEG();
    ImageErrorCode readFile( Image *img, const String &filename );
    ImageErrorCode readData( Image *img, const Byte *data, int size );
  };
  
  class ImageEncoderJPEG : public ImageEncoder
  {
  public:
    ImageEncoderJPEG();
    ImageErrorCode writeFile( Image *img, const String &filename, void *params );
  };
  
}

#endif /* __USEJPEG_H */
