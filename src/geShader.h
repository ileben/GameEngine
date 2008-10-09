#ifndef __GE_SHADER_H
#define __GE_SHADER_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  //forward declarations
  class Material;
  
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
      String      name;
      UniformType type;
      int count;
      
      Uniform () {};
      Uniform (const String &newName, UniformType newType, int newCount)
        { name = newName; type = newType; count = newCount; }
    };
    
    GLShader  *vertex;
    GLShader  *fragment;
    GLProgram *program;
    ArrayList<Uniform> uniforms;
    
    void freeProgram ();
    
  public:
    
    Shader ();
    virtual ~Shader ();
    
    void fromFile (const String &fileVertex,
                   const String &fileFragment);
    
    void registerUniform (const String &name,
                          UniformType type,
                          int count);
    
    UintSize getUniformCount ();
    Uniform& getUniform (UintSize index);
    void use ();
  };
}

#pragma warning(pop)
#endif //__GE_SHADER_H
