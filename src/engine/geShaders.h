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
    CharString getInfoLog ();
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
    CharString getInfoLog ();
    void use ();
    static void UseFixed ();
    void bindAttribute (Uint32 index, const char *name);
    Int32 getUniform (const char *name) const;
    Int32 getAttribute (const char *name) const;
  };

}/* namespace GE */

#endif /* __GESHADERS_H */
