#ifndef __GESHADERS_H
#define __GESHADERS_H

#include "util/geUtil.h"

namespace GE
{
  /*================================
   * Forward declarations
   *================================*/

  class GLProgram;

  /*================================
   * Vertex or fragment shader
   *================================*/

  namespace ShaderType {
    enum Enum
    {
      Vertex     =  0,
      Fragment   =  1,
      Any        =  2
    };};

  class GLShader : public Object
  {
    CLASS( GLShader, Object,
      cac5feda,8e46,4279,a10cbe04e7c51b92 );
    
  protected:
    friend class GLProgram;
    Uint32 handle;
    ShaderType::Enum type;
    
  public:
    GLShader ();
    virtual ~GLShader ();
    void create (ShaderType::Enum type);
    bool compile (const CharString &source);
    CharString getInfoLog ();
  };

  /*==============================
   * Shading program
   *==============================*/

  class GLProgram : public Object
  {
    CLASS( GLProgram, Object,
      e01dce7d,6d00,4ee5,914d6cba969bec91 );
    
  protected:
    Uint32 handle;
   
  public:
    GLProgram ();
    virtual ~GLProgram ();
    
    void create ();
    void attach (GLShader *s);
    void detach (GLShader *s);

    bool link ();
    CharString getInfoLog ();
    void use ();
    static void UseFixed ();
    void bindAttribute (Uint32 index, const char *name);
    Int32 getUniform (const char *name) const;
    Int32 getAttribute (const char *name) const;
  };

}/* namespace GE */

#endif /* __GESHADERS_H */
