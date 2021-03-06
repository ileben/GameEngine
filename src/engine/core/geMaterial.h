#ifndef __GEMATERIAL_H
#define __GEMATERIAL_H

#pragma warning(push)
#pragma warning(disable:4251)

#include "util/geUtil.h"
#include "math/geMath.h"
#include "core/geKernel.h"
#include "core/geResource.h"

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
  
  typedef Uint32 MaterialID;
  #define GE_MAX_MATERIAL_ID 59999
  #define GE_ANY_MATERIAL_ID 60000
  
  /*
  ============================================
  
  Generic Material interface
  
  ============================================*/
  
  class Material : public Resource
  {
    CLASS( Material, Resource,
      624cc67c,8c0d,4da6,927f667e529bb996 );

    friend class Renderer;
    friend class MultiMaterial;
    
  public:

    virtual Class getShaderComposingClass() { return ClassName( Material ); }
    virtual void composeShader( Shader *shader ) {}
    
    Material () {}
    virtual ~Material () {}
    
    virtual void begin () {}
    virtual void end () {}

    virtual void beginShadow() {}
    virtual void endShadow() {}
    
    static void BeginDefault ();
    static void EndDefault ();
  };
  
  
  /*
  ============================================
  
  Standard material (fixed functionality)
  
  ============================================*/
  
  class StandardMaterial : public Material
  {
    CLASS( StandardMaterial, Material,
      c2b106d9,8973,43ed,85a8ca336dd70d75 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Material::serialize( s, v );
      s->data( &diffuseColor );
      s->data( &ambientColor );
      s->data( &specularColor );
      s->data( &specularity );
      s->data( &glossiness );
      s->data( &opacity );
      s->data( &luminosity );
      s->data( &lighting );
      s->data( &culling );
      s->data( &cell );
    }

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
    bool wire;

    Int32 uLuminosity;
    Int32 uSpecularity;
    Int32 uCellShading;
    bool gotUniforms;

  public:

    virtual Class getShaderComposingClass() { return ClassName( StandardMaterial ); }
    virtual void composeShader( Shader *shader );
    
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

    void setWireframe (bool enable);
    bool getWireframe ();
    
    virtual void begin();
    virtual void end();

    virtual void beginShadow();
    virtual void endShadow();
  };

  /*
  ============================================
  
  Uses a texture for diffuse color
  
  ============================================*/

  class DiffuseTexMat : public StandardMaterial
  {
    CLASS( DiffuseTexMat, StandardMaterial,
      efb36eb3,5397,4cdc,af19db7f9b1e6759 );

    virtual void serialize( Serializer *s, Uint v )
    {
      StandardMaterial::serialize( s, v );
      s->object( &texDiffuse );
    }

  private:

    TextureRef texDiffuse;
    Int32 uDiffSampler;
    bool gotUniforms;

  public:

    virtual Class getShaderComposingClass() { return ClassName( DiffuseTexMat ); }
    virtual void composeShader (Shader *shader);

    DiffuseTexMat ();

    void setDiffuseTexture (Texture *tex);
    void setDiffuseTexture (const CharString &name);
    Texture *getDiffuseTexture ();

    virtual void begin();
    virtual void end();
  };

  class NormalTexMat : public DiffuseTexMat
  {
    CLASS( NormalTexMat, DiffuseTexMat,
      61aa1181,a126,429a,881115d72ca6250e );

    virtual void serialize( Serializer *s, Uint v )
    {
      DiffuseTexMat::serialize( s,v );
      s->object( &texNormal );
    }

  private:

    TextureRef texNormal;
    Int32 uNormSampler;
    bool gotUniforms;

  public:

    virtual Class getShaderComposingClass() { return ClassName( NormalTexMat ); }
    virtual void composeShader( Shader *shader );
    
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
    CLASS( MultiMaterial, Material,
      65c8d297,17a8,4793,b1a6d21ca5bec028 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Material::serialize( s,v );
      s->objectPtrArray( &subMaterials );
    }

    friend class Renderer;
    
  private:
    UintSize selectedID;
    ArrayList <Material*> subMaterials;
    bool selectSubMaterial( MaterialID id );
    bool selectionValid();
    
  public:
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
