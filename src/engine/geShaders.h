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
    DECLARE_SUBCLASS( GLShader, Object );
    DECLARE_END;

    friend class GLProgram;
    
  protected:
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
    DECLARE_SUBCLASS( GLProgram, Object);
    DECLARE_END;
    
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
