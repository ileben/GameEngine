#ifndef __GETEXTURE_H
#define __GETEXTURE_H

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
    OCC::ColorFormat format;

  public:
    Texture();
    ~Texture();

    Uint32 getHandle();
    OCC::ColorFormat getFormat();

    void fromData(int width, int height, OCC::ColorFormat format, const void *data);
    void fromImage(const OCC::Image *img);

    void updateRegion(int offX, int offY, int width, int height, OCC::ColorFormat format, const void *data);
    void updateRegion(int offX, int offY, const OCC::Image *img);
  };
}

#endif /* __GETEXTURE_H */
