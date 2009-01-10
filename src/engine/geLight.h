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
  };
  
  class DirLight : public Light
  {
  protected:
    Vector3 direction;
    
  public:
    DirLight () {}
    DirLight (const Vector3 &dir);
    void setDirection (const Vector3 &dir);
    virtual void enable (int index);
  };
  
  class PointLight : public Light
  {
  protected:
    Vector3 position;

  public:
    PointLight () {}
    PointLight (const Vector3 &pos);
    void setPosition (const Vector3 &pos);
    virtual void enable (int index);
  };
  
  class SpotLight : public Light
  {
  protected:
    Vector3 position;
    Vector3 direction;
    Float angle;

  public:
    SpotLight () : direction( 0.0f,0.0f,1.0f ), angle( 45.0f ) {}
    SpotLight (const Vector3 &pos, const Vector3 &dir, Float angle);
    void setPosition (const Vector3 &pos);
    void setDirection (const Vector3 &dir);
    void setAngle (Float angle);
    virtual void enable (int index);
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
