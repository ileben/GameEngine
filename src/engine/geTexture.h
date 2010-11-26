#ifndef __GETEXTURE_H
#define __GETEXTURE_H

#include "util/geUtil.h"
#include "image/geImage.h"
#include "geResource.h"

namespace GE
{
  class Renderer;

  class Texture : public Resource
  {
    CLASS( Texture, Resource,
      48373fd3,f969,45a1,ab0359580eb7be8e );

  private:
    friend class Renderer;
    friend class Material;

    Uint32 handle;
    ColorFormat format;

  public:
    Texture();
    ~Texture();

    Uint32 getHandle();
    ColorFormat getFormat();

    void fromData(int width, int height, ColorFormat format, const void *data);
    void fromImage(const Image *img);

    void updateRegion(int offX, int offY, int width, int height, ColorFormat format, const void *data);
    void updateRegion(int offX, int offY, const Image *img);
  };
}

#endif /* __GETEXTURE_H */
