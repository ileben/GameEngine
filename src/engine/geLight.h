#ifndef __GELIGHT_H
#define __GELIGHT_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"
#include "geActor.h"

namespace GE
{

  class Light : public Actor3D
  {
    CLASS( Light, a55ed88c,c0b5,4f90,b04805dcdc32975f );
    virtual void serialize( Serializer *s, Uint v )
    {
      Actor3D::serialize( s,v );
      s->data( &shadowsOn );
      s->data( &diffuseColor );
      s->data( &specularColor );
      s->data( &shadowColor );
      s->data( &attStart );
      s->data( &attEnd );
    }
    
  protected:
    bool shadowsOn;
    Vector3 diffuseColor;
    Vector3 specularColor;
    Vector3 shadowColor;
    Float attStart;
    Float attEnd;

    bool uniformsInit;
    Int32 uSampler[5];
    Int32 uCastShadow;
    Int32 uWinSize;
    Int32 uAttEnd;
    Int32 uAttStart;

    bool volumeDLinit;
    bool volumeChanged;
    Uint32 volumeDL;
    virtual void updateVolume() {};
    
  public:
    Light ();
    virtual ~Light() {};

    void setDirection (const Vector3 &dir);
    void setPosition (const Vector3 &dir);
    Vector3 getDirection ();
    Vector3 getPosition ();
    
    virtual void setCastShadows (bool cast);
    bool getCastShadows ();

    virtual void setDiffuseColor (const Vector3 &color);
    const Vector3& getDiffuseColor ();

    virtual void setSpecularColor (const Vector3 &color);
    const Vector3& getSpecularColor ();

    virtual void setShadowColor (const Vector3 &color);
    const Vector3& getShadowColor ();

    virtual void setAttenuation (Float end, Float start = -1.0f);
    Float getAttenuationEnd ();
    Float getAttenuationStart ();

    virtual Matrix4x4 getProjection ();
    virtual bool isPointInVolume (const Vector3 &p, Float threshold=0.0f) { return false; }
    void renderVolume ();

    virtual void composeShader (Shader *shader);
    virtual void enable (int index);
    virtual void begin (Shader *shader, Vector2 winSize, Uint32 *deferredMaps, Uint32 shadowMap);
    virtual void end();
  };
  
  class DirLight : public Light
  {
    CLASS( DirLight, 816d73cb,bb3c,4e2d,94b8dc96a35eee34 );

  public:
    DirLight () {}
    DirLight (const Vector3 &dir);
    virtual void enable (int index);
  };
  
  class SpotLight : public Light
  {
    CLASS( SpotLight, bb3c8a4f,8b92,40bd,83043b7ea071af74 );
    virtual void serialize (Serializer *s, Uint v)
    {
      Light::serialize( s,v );
      s->data( &angleInner );
      s->data( &angleOuter );
    }

  protected:
    Float angleInner;
    Float angleOuter;
    virtual void updateVolume ();

  public:
    SpotLight () : angleOuter( 30.0f ), angleInner( -1.0f ) {}
    SpotLight (const Vector3 &pos,
               const Vector3 &dir,
               Float outerAngle = 60.0f,
               Float innerAngle = -1.0f);
    
    void setAngle (Float outer, Float inner = -1.0f);
    virtual void enable (int index);
    virtual Matrix4x4 getProjection ();
    virtual bool isPointInVolume (const Vector3 &p, Float threshold=0.0f);

    virtual void composeShader (Shader *shader);
  };

  class PyramidLight : public Light
  {
    CLASS( PyramidLight, a46d282c,8492,4dd4,a6100e93a09c6ba3 );
    virtual void serialize( Serializer *s, Uint v )
    {
      Light::serialize( s,v );
      s->data( &angle );
    }

  protected:

    Float angle;
    virtual void updateVolume ();

  public:

    PyramidLight () : angle (60.0f) {}
    PyramidLight (const Vector3 &pos,
                  const Vector3 &dir,
                  Float angle = 60.0f);

    void setAngle (Float a);
    virtual void enable (int index);
    virtual Matrix4x4 getProjection ();
    virtual bool isPointInVolume (const Vector3 &p, Float threshold=0.0f);

    virtual void composeShader (Shader *shader);
  };

  class PointLight : public Light
  {
    CLASS( PyramidLight, e684697c,c0a1,446b,927069f5eff3ad37 );

    PointLight (bool producing) {}
    static PointLight* produce() { return new PointLight(true); }
    virtual void serialize( Serializer *s, Uint v )
    {
      Light::serialize( s,v );
      s->objectRefArray( &subLights );
    }
    
  private:
    ArrayList< PyramidLight* > subLights;
    
  public:
    PointLight ();
    PointLight (const Vector3 &pos);

    //Overrides that apply to each sub-light
    void setCastShadows (bool cast);
    void setDiffuseColor (const Vector3 &color);
    void setSpecularColor (const Vector3 &color);
    void setShadowColor (const Vector3 &color);
    void setAttenuation (Float end, Float start = -1.0f);
  };
  
  class HeadLight : public PointLight
  {
    CLASS( HeadLight, 2467c1e8,bf8f,4e70,a63e32e47d5f07d6 );

  public:
    HeadLight () {}
    HeadLight (const Vector3 &pos) : PointLight (pos) {}
    virtual void enable (int index);
  };

}//namespace GE
#endif//__GELIGHT_H
