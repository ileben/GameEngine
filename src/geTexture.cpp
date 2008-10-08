#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using OCC::ColorFormat;
using OCC::Image;

namespace GE
{
  DEFINE_CLASS (Texture);
  
  
  Texture::Texture()
  {
    glGenTextures(1, (GLuint*)&handle);

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  Texture::~Texture()
  {
    glDeleteTextures(1, (GLuint*)&handle);
  }

  void Texture::fromData(int width, int height, enum ColorFormat format, const void *data)
  {
    this->format = format;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, handle);

    switch(format)
    {
    case OCC::COLOR_FORMAT_GRAY:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_GRAY_ALPHA: 
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_RGB:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_RGB_ALPHA:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      break;
    }
  }
  
  void Texture::fromImage(const Image *img)
  {
    fromData(img->getWidth(), img->getHeight(), img->getFormat(), img->getData());
  }

  void Texture::updateRegion(int offX, int offY, int width, int height, ColorFormat format, const void *data)
  {
    this->format = format;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, handle);

    switch(format)
    {
    case OCC::COLOR_FORMAT_GRAY:
      glTexSubImage2D(GL_TEXTURE_2D, 0, offX, offY, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_GRAY_ALPHA: 
      glTexSubImage2D(GL_TEXTURE_2D, 0, offX, offY, width, height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_RGB:
      glTexSubImage2D(GL_TEXTURE_2D, 0, offX, offY, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
      break;

    case OCC::COLOR_FORMAT_RGB_ALPHA:
      glTexSubImage2D(GL_TEXTURE_2D, 0, offX, offY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
      break;
    }
  }

  void Texture::updateRegion(int offX, int offY, const Image *img)
  {
    updateRegion(offX, offY, img->getWidth(), img->getHeight(), img->getFormat(), img->getData());
  }

  Uint32 Texture::getHandle()
  {
    return handle;
  }
  
  ColorFormat Texture::getFormat()
  {
    return format;
  }

}/* namespace GE */
