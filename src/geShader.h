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
      OCC::String name;
      UniformType type;
      int count;
      
      Uniform () {};
      Uniform (const OCC::String &newName, UniformType newType, int newCount)
        { name = newName; type = newType; count = newCount; }
    };
    
    GLShader  *vertex;
    GLShader  *fragment;
    GLProgram *program;
    OCC::ArrayList<Uniform> uniforms;
    
    void freeProgram ();
    
  public:
    
    Shader ();
    virtual ~Shader ();
    
    void fromFile (const OCC::String &fileVertex,
                   const OCC::String &fileFragment);
    
    void registerUniform (const OCC::String &name,
                          UniformType type,
                          int count);
    
    int getUniformCount ();
    Uniform& getUniform (int index);
    void use ();
  };
}

#pragma warning(pop)
#endif //__GE_SHADER_H
