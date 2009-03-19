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

  enum ShaderType
  {
    GE_SHADER_INVALID,
    GE_SHADER_VERTEX,
    GE_SHADER_FRAGMENT
  };

  class GE_API_ENTRY GLShader
  {
    DECLARE_CLASS (GLShader); DECLARE_END;
    friend class GLProgram;
    
  protected:
    Uint32 handle;
    ShaderType type;
    
  public:
    GLShader ();
    virtual ~GLShader ();
    void create (ShaderType type);
    void fromString (const char *code);
    bool fromFile (const String &file);
    bool compile ();
    char* getInfoLog ();
  };

  /*==============================
   * Shading program
   *==============================*/

  class GE_API_ENTRY GLProgram
  {
    DECLARE_CLASS (GLProgram); DECLARE_END;
    
  protected:
    Uint32 handle;
    GLShader *vertex;
    GLShader *fragment;
   
  public:
    GLProgram ();
    virtual ~GLProgram ();
    void create ();
    void attach (GLShader *s);
    void detach (ShaderType which);
    GLShader* getVertex ();
    GLShader* getFragment ();
    bool link ();
    char* getInfoLog ();
    void use ();
    static void UseFixed ();
    Int32 getUniform (const char *name) const;
  };

}/* namespace GE */

#endif /* __GESHADERS_H */
