#ifndef __GE_SHADER_H
#define __GE_SHADER_H

#include "util/geUtil.h"
#include "geResource.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/
  class GLShader;
  class GLProgram;

  /*
  ------------------------------------
  Shader
  ------------------------------------*/
  
  enum UniformType
  {
    GE_UNIFORM_INT,
    GE_UNIFORM_FLOAT,
    GE_UNIFORM_MATRIX,
    GE_UNIFORM_TEXTURE
  };
  
  class GE_API_ENTRY Shader : public Resource
  {
    DECLARE_SUBCLASS (Shader, Resource); DECLARE_END;
    friend class Material;
    
  private:
    
    struct Uniform
    {
      CharString  name;
      UniformType type;
      int count;
      Int32 ID;
      
      Uniform () {};
      Uniform (const CharString &newName, UniformType newType, int newCount)
        { name = newName; type = newType; count = newCount; }
    };

    struct VertexAttrib
    {
      CharString name;
      Int32 ID;

      VertexAttrib() {};
      VertexAttrib(const CharString &newName)
        { name = newName; }
    };
    
    GLShader  *vertex;
    GLShader  *fragment;
    GLProgram *program;
    ArrayList<Uniform> uniforms;
    ArrayList<VertexAttrib> attribs;
    
    void freeProgram ();
    
  public:
    
    Shader ();
    virtual ~Shader ();

    void registerVertexAttrib (const CharString &name);
    
    void registerUniform (const CharString &name,
                          UniformType type,
                          int count);

    Int32 getVertexAttribID (UintSize index);
    Int32 getVertexAttribID (const CharString &name);
    Int32 getUniformID (UintSize index);
    Int32 getUniformID (const CharString &name);

    void fromFile (const String &fileVertex,
                   const String &fileFragment);
    
    UintSize getUniformCount ();
    Uniform& getUniform (UintSize index);
    const GLProgram* getGLProgram ();
    void use ();
  };
}

#pragma warning(pop)
#endif //__GE_SHADER_H
