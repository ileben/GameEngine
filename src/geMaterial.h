#ifndef __GEMATERIAL_H
#define __GEMATERIAL_H

#pragma warning(push)
#pragma warning(disable:4251)

#include "geShaders.h"

namespace GE
{
  /*
  -----------------------------------------
  Forward declarations
  -----------------------------------------*/
  class Renderer;
  class Shader;

  /*
  =========================================================
  
  MaterialId is an index into sub-material array.
  Max value is clamped to one less then max representable
  so it is possible to hit the end value when iterating
  over the array using MaterialId type.
  
  =========================================================*/
  
  typedef Uint8 MaterialId;
  #define GE_MAX_MATERIAL_ID 254
  #define GE_ANY_MATERIAL_ID 255
  
  /*
  ============================================
  
  Generic Material interface
  
  ============================================*/
  
  class GE_API_ENTRY Material : public Resource
  {
    DECLARE_SUBABSTRACT (Material, Resource); DECLARE_END;
    friend class Renderer;
    friend class MultiMaterial;
    
  protected: //user subclasses
    
    /*
    ============================================
    
    Property - material property interface
    
    ============================================*/
      
    class UniformProperty
    {
      //DECLARE_SUBABSTRACT (UniformProperty, Property);
    protected:
      OCC::CharString name;
      
    public:
      const OCC::CharString& getName ();
      virtual ~UniformProperty ();
      virtual void set (void *value) = 0;
      virtual void begin () = 0;
      virtual void end () {}
    };
    
    /*
    =================================================
    
    UniformVecProperty - material property that
    sets up vector uniform variables of the shader
    
    =================================================*/
    
    template <class T>
    class UniformVecProperty : public UniformProperty
    {
      //DECLARE_SUBCLASS (UniformVecProperty, UniformProperty);
      
    private:
      T          *val;
      int         count;
      GLProgram  *program;
      void freeval ();
      
    public:
      UniformVecProperty () {};
      virtual ~UniformVecProperty ();
      UniformVecProperty (GLProgram *program,
                          const OCC::String &name,
                          int count);
      
      virtual void set (void *value);
      virtual void begin ();
      virtual void end ();
    };
    
    /*
    ============================================================
    
    UniformTexProperty - material property that passes current
    texture unit to a shader's sampler uniform variable and
    sets up GL texturing state.
    
    ============================================================*/
    
    class UniformTexProperty : public UniformProperty
    {
      //DECLARE_SUBCLASS (UniformTexProperty, UniformProperty);
      
    private:
      Texture    *val;
      int         texunit;
      GLProgram  *program;
      
    public:
      UniformTexProperty () {}
      UniformTexProperty (GLProgram *program,
                          const OCC::String &name,
                          int texture);
      
      virtual void set (void *value);
      virtual void begin ();
      virtual void end ();
    };
    
  private: //internal classes
    
    Shader *shader;
    OCC::ArrayList <UniformProperty*> uniformProps;
    void freeUniformProps ();
    
  public: //user
    
    Material ();
    virtual ~Material ();
    
    void setShader (Shader *newShader);
    void setProperty (const OCC::String &name, void *value);
    
    virtual void begin ();
    virtual void end ();
    
    static void BeginDefault ();
    static void EndDefault ();
  };
  
  
  /*
  ============================================
  
  Standard material (fixed functionality)
  
  ============================================*/
  
  class GE_API_ENTRY StandardMaterial : public Material
  {
    DECLARE_SUBCLASS (StandardMaterial, Material); DECLARE_END;
    friend class Renderer;
    
  private:
    
    Vector3 diffuseColor;
    Vector3 ambientColor;
    Vector3 specularColor;
    Float specularity;
    Float glossiness;
    Float opacity;
    bool lighting;
    bool culling;

  public:
    
    StandardMaterial();
    
    void setDiffuseColor (const Vector3 &color);
    const Vector3& getDiffuseColor ();

    void setAmbientColor (const Vector3 &color);
    const Vector3& getAmbientColor ();
    
    void setSpecularColor (const Vector3 &color);
    const Vector3& getSpecularColor ();
    
    void setOpacity (Float opacity);
    Float getOpacity ();
    
    void setSpecularity (Float spec);
    Float getSpecularity ();

    void setGlossiness (Float gloss);
    Float getGlossiness ();

    void setUseLighting (bool enable);
    bool getUseLighting ();

    void setCullBack (bool enable);
    bool getCullBack ();
    
    virtual void begin();
  };
  
  /*
  ==================================================
  
  Multi-material consists of several sub-materials
  to be referenced via a per-face material ID.
  
  ==================================================*/
  
  class GE_API_ENTRY MultiMaterial : public Material
  {
    DECLARE_SUBCLASS (MultiMaterial, Material); DECLARE_END;
    friend class Renderer;
    
  private:
    int selectedId;
    OCC::ArrayList<Material*> subMaterials;
    bool selectSubMaterial (MaterialId id);
    bool selectionValid ();
    
  public:
    void setNumSubMaterials (int n);
    void setSubMaterial (MaterialId id, Material *m);
    Material* getSubMaterial (MaterialId id);
    int getNumSubMaterials ();
    
    virtual void begin ();
    virtual void end ();
  };
  
  /*
  ============================================
  
  Per-pixel phong-shaded material
  
  ============================================*/
  
  class GE_API_ENTRY PhongMaterial : public StandardMaterial
  {
    DECLARE_SUBCLASS (PhongMaterial, StandardMaterial);
    DECLARE_END;
    
  private:
    
    Texture *texDiffuse;
    Texture *texSpecularity;
    
  public:
    
    PhongMaterial ();
    
    void setDiffuseTexture (Texture *tex);
    Texture *getDiffuseTexture ();
    
    void setSpecularityTexture (Texture *tex);
    Texture *getSpecularityTexture ();
    
    virtual void begin ();
  };
  
  
}//namespace GE
#pragma warning(pop)
#endif// __GEMATERIAL_H
