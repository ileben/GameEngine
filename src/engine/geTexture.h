#ifndef __GETEXTURE_H
#define __GETEXTURE_H

#include "util/geUtil.h"
#include "image/geImage.h"
#include "geResource.h"

namespace GE
{
  class Renderer;

  class GE_API_ENTRY Texture : public Resource
  {
    DECLARE_SUBCLASS (Texture, Resource); DECLARE_END;
    friend class Renderer;
    friend class Material;

  private:
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
