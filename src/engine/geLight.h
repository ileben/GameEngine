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
    DECLARE_SERIAL_SUBCLASS( Light, Actor3D );
    DECLARE_DATAVAR( shadowsOn );
    DECLARE_DATAVAR( diffuseColor );
    DECLARE_DATAVAR( specularColor );
    DECLARE_DATAVAR( shadowColor );
    DECLARE_DATAVAR( attStart );
    DECLARE_DATAVAR( attEnd );
    DECLARE_END;
    
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
    Light (SM *sm);
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
    DECLARE_SUBCLASS( DirLight, Light );
    DECLARE_END;

  public:
    DirLight () {}
    DirLight (const Vector3 &dir);
    virtual void enable (int index);
  };
  
  class SpotLight : public Light
  {
    DECLARE_SERIAL_SUBCLASS( SpotLight, Light );
    DECLARE_DATAVAR( angleInner );
    DECLARE_DATAVAR( angleOuter );
    DECLARE_END;

  protected:
    Float angleInner;
    Float angleOuter;
    virtual void updateVolume ();

  public:
    SpotLight (SM *sm) : Light(sm) {}
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
    DECLARE_SERIAL_SUBCLASS( PyramidLight, Light );
    DECLARE_DATAVAR( angle );
    DECLARE_END;

  protected:
    Float angle;
    virtual void updateVolume ();

  public:

    PyramidLight (SM *sm) : Light (sm) {}
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
    DECLARE_SERIAL_SUBCLASS( PointLight, Light );
    DECLARE_OBJVAR( subLights );
    DECLARE_END;
    
    ObjRefArrayList< PyramidLight > subLights;
    
  public:

    PointLight (SM *sm) : Light (sm) {}
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
    DECLARE_SUBCLASS( HeadLight, Light );
    DECLARE_END;

  public:
    HeadLight () {}
    HeadLight (const Vector3 &pos) : PointLight (pos) {}
    virtual void enable (int index);
  };

}//namespace GE
#endif//__GELIGHT_H
