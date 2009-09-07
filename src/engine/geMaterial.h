#ifndef __GEMATERIAL_H
#define __GEMATERIAL_H

#pragma warning(push)
#pragma warning(disable:4251)

#include "util/geUtil.h"
#include "math/geMath.h"
#include "engine/geKernel.h"
#include "engine/geResource.h"

namespace GE
{
  /*
  -----------------------------------------
  Forward declarations
  -----------------------------------------*/
  class Renderer;
  class Shader;
  class Texture;
  class GLProgram;

  /*
  =========================================================
  
  MaterialID is an index into sub-material array.
  Max value is clamped to one less then max representable
  so it is possible to hit the end value when iterating
  over the array using MaterialID type.
  
  =========================================================*/
  
  typedef Uint8 MaterialID;
  #define GE_MAX_MATERIAL_ID 254
  #define GE_ANY_MATERIAL_ID 255
  
  /*
  ============================================
  
  Generic Material interface
  
  ============================================*/
  
  class Material : public Resource
  {
    DECLARE_SUBABSTRACT( Material, Resource );
    DECLARE_END;

    friend class Renderer;
    friend class MultiMaterial;
    
  public:

    virtual ClassPtr getShaderComposingClass() { return Class(Material); }
    virtual void composeShader( Shader *shader ) {}
    
    Material () {}
    virtual ~Material () {}
    
    virtual void begin () {}
    virtual void end () {}
    
    static void BeginDefault ();
    static void EndDefault ();
  };
  
  
  /*
  ============================================
  
  Standard material (fixed functionality)
  
  ============================================*/
  
  class StandardMaterial : public Material
  {
    DECLARE_SERIAL_SUBCLASS( StandardMaterial, Material );
    DECLARE_DATAVAR( diffuseColor );
    DECLARE_DATAVAR( ambientColor );
    DECLARE_DATAVAR( specularColor );
    DECLARE_DATAVAR( specularity );
    DECLARE_DATAVAR( glossiness );
    DECLARE_DATAVAR( opacity );
    DECLARE_DATAVAR( luminosity );
    DECLARE_DATAVAR( lighting );
    DECLARE_DATAVAR( culling );
    DECLARE_DATAVAR( cell );
    DECLARE_END;

    friend class Renderer;
    
  protected:
    
    Vector3 diffuseColor;
    Vector3 ambientColor;
    Vector3 specularColor;
    Float specularity;
    Float glossiness;
    Float opacity;
    Float luminosity;
    bool lighting;
    bool culling;
    bool cell;

    Int32 uLuminosity;
    Int32 uSpecularity;
    Int32 uCellShading;
    bool gotUniforms;

  public:

    virtual ClassPtr getShaderComposingClass() { return Class(StandardMaterial); }
    virtual void composeShader( Shader *shader );
    
    StandardMaterial (SM *sm);
    StandardMaterial ();
    
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

    void setLuminosity (Float luminosity);
    Float getLuminosity ();

    void setUseLighting (bool enable);
    bool getUseLighting ();

    void setCullBack (bool enable);
    bool getCullBack ();

    void setCellShaded (bool enable);
    bool getCellShaded ();
    
    virtual void begin();
  };

  /*
  ============================================
  
  Uses a texture for diffuse color
  
  ============================================*/

  class DiffuseTexMat : public StandardMaterial
  {
    DECLARE_SERIAL_SUBCLASS( DiffuseTexMat, StandardMaterial );
    DECLARE_OBJVAR( texDiffuse );
    DECLARE_END;

  private:

    TextureRef texDiffuse;
    Int32 uDiffSampler;
    bool gotUniforms;

  public:

    virtual ClassPtr getShaderComposingClass() { return Class(DiffuseTexMat); }
    virtual void composeShader (Shader *shader);

    DiffuseTexMat (SM *sm);
    DiffuseTexMat ();

    void setDiffuseTexture (Texture *tex);
    void setDiffuseTexture (const CharString &name);
    Texture *getDiffuseTexture ();

    virtual void begin();
    virtual void end();
  };

  class NormalTexMat : public DiffuseTexMat
  {
    DECLARE_SERIAL_SUBCLASS (NormalTexMat, DiffuseTexMat);
    DECLARE_OBJVAR( texNormal );
    DECLARE_END;

  private:

    TextureRef texNormal;
    Int32 uNormSampler;
    bool gotUniforms;

  public:

    virtual ClassPtr getShaderComposingClass() { return Class(NormalTexMat); }
    virtual void composeShader( Shader *shader );

    NormalTexMat (SM *sm);
    NormalTexMat ();

    void setNormalTexture (Texture *tex);
    void setNormalTexture (const CharString &name);
    Texture *getNormalTexture ();

    virtual void begin();
    virtual void end();
  };
  
  /*
  ==================================================
  
  Multi-material consists of several sub-materials
  to be referenced via a per-face material ID.
  
  ==================================================*/
  
  class MultiMaterial : public Material
  {
    DECLARE_SERIAL_SUBCLASS (MultiMaterial, Material);
    DECLARE_OBJVAR (subMaterials);
    DECLARE_END;
    friend class Renderer;
    
  private:
    UintSize selectedID;
    ObjPtrArrayList <Material> subMaterials;
    bool selectSubMaterial( MaterialID id );
    bool selectionValid();
    
  public:
    MultiMaterial (SM *sm);
    MultiMaterial ();

    void setNumSubMaterials( UintSize n );
    void setSubMaterial( MaterialID id, Material *m );
    Material* getSubMaterial( MaterialID id );
    UintSize getNumSubMaterials();
    
    virtual void begin ();
    virtual void end ();
  };
  
  
}//namespace GE
#pragma warning(pop)
#endif// __GEMATERIAL_H
