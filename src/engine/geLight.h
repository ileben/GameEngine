#ifndef __GELIGHT_H
#define __GELIGHT_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"
#include "geActor.h"

namespace GE
{

  class Light : public Actor
  {
    DECLARE_SUBCLASS( Light, Actor ); DECLARE_END;
    
  protected:
    bool shadowsOn;
    Vector3 diffuseColor;
    Vector3 specularColor;
    Vector3 shadowColor;

    Float attenuationEnd;
    bool volumeDLinit;
    Uint32 volumeDL;
    virtual void updateVolume() {};
    
  public:
    Light();
    
    void setCastShadows (bool cast);
    bool getCastShadows ();

    void setDiffuseColor (const Vector3 &color);
    const Vector3& getDiffuseColor ();

    void setSpecularColor (const Vector3 &color);
    const Vector3& getSpecularColor ();

    void setShadowColor (const Vector3 &color);
    const Vector3& getShadowColor ();

    void setDirection (const Vector3 &dir);
    void setPosition (const Vector3 &dir);
    Vector3 getDirection ();
    Vector3 getPosition ();

    void setAttenuationEnd (Float end);
    Float getAttenuationEnd ();

    virtual void enable (int index);
    virtual Matrix4x4 getProjection (Float nearClip, Float farClip);
    virtual bool isPointInVolume (const Vector3 &p, Float threshold=0.0f) { return false; }
    void renderVolume ();
  };
  
  class DirLight : public Light
  {
    DECLARE_SUBCLASS( DirLight, Light ); DECLARE_END;

  public:
    DirLight () {}
    DirLight (const Vector3 &dir);
    virtual void enable (int index);
  };
  
  class PointLight : public Light
  {
    DECLARE_SUBCLASS( PointLight, Light ); DECLARE_END;

  public:
    PointLight () {}
    PointLight (const Vector3 &pos);
    virtual void enable (int index);
  };
  
  class SpotLight : public Light
  {
    DECLARE_SUBCLASS( SpotLight, Light ); DECLARE_END;

  protected:
    Float angleInner;
    Float angleOuter;
    virtual void updateVolume ();

  public:
    SpotLight () : angleOuter( 30.0f ), angleInner( -1.0f ) {}
    SpotLight (const Vector3 &pos,
               const Vector3 &dir,
               Float outerAngle = 30.0f,
               Float innerAngle = -1.0f);
    
    void setAngle (Float outer, Float inner = -1.0f);
    virtual void enable (int index);
    virtual Matrix4x4 getProjection (Float nearClip, Float farClip);
    virtual bool isPointInVolume (const Vector3 &p, Float threshold=0.0f);
  };
  
  class HeadLight : public PointLight
  {
    DECLARE_SUBCLASS( HeadLight, Light ); DECLARE_END;

  public:
    HeadLight () {}
    HeadLight (const Vector3 &pos) : PointLight (pos) {}
    virtual void enable (int index);
  };

}//namespace GE
#endif//__GELIGHT_H
