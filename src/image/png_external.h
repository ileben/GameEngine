#ifndef __USEPNG_H
#define __USEPNG_H

namespace GE
{

  class ImageDecoderPNG : public ImageDecoder
  {
  public:
    ImageDecoderPNG();
    ImageErrorCode readFile(Image *img, const String &filename);
    ImageErrorCode readData(Image *img, const BYTE *data, int size);
  };
  
}

#endif /* __USEPNG_H */
