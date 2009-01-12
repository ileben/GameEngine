#ifndef __GELIGHT_H
#define __GELIGHT_H

namespace GE
{

  class Light : public Actor
  {
  private:
    virtual RenderRole::Enum getRenderRole();
    
  public:
    Light();
    void setDirection (const Vector3 &dir);
    void setPosition (const Vector3 &dir);
    virtual void enable (int index);
    virtual Matrix4x4 getProjection (Float nearClip, Float farClip);
  };
  
  class DirLight : public Light
  {
  public:
    DirLight () {}
    DirLight (const Vector3 &dir);
    virtual void enable (int index);
  };
  
  class PointLight : public Light
  {
  public:
    PointLight () {}
    PointLight (const Vector3 &pos);
    virtual void enable (int index);
  };
  
  class SpotLight : public Light
  {
  protected:
    Float angleInner;
    Float angleOuter;
    
  public:
    SpotLight () : angleOuter( 30.0f ), angleInner( -1.0f ) {}
    SpotLight (const Vector3 &pos,
               const Vector3 &dir,
               Float outerAngle = 30.0f,
               Float innerAngle = -1.0f);
    
    void setAngle (Float outer, Float inner = -1.0f);
    virtual void enable (int index);
    virtual Matrix4x4 getProjection (Float nearClip, Float farClip);
  };
  
  class HeadLight : public PointLight
  {
  public:
    HeadLight () {}
    HeadLight (const Vector3 &pos) : PointLight (pos) {}
    virtual void enable (int index);
  };

}//namespace GE
#endif//__GELIGHT_H
