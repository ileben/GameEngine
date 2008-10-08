#ifndef __GEACTOR_H
#define __GEACTOR_H

namespace GE
{
  /*
  -------------------------------------------------
  Actor represents a single node to be placed
  in the scene tree. It can be a shape, camera
  or any other kind of actor (derived from the
  base). Transformation matrix allows for
  additional transformation of the object content
  -------------------------------------------------*/
  
  class GE_API_ENTRY Actor : public Object
  {
    DECLARE_SUBCLASS (Actor, Object); DECLARE_END;
    friend class Group;
    
  private:
    
    Matrix4x4 actor2world;
    Material *material;

  protected:
    
    Group *parent;
    Float mass;    
    
  public:
    
    Actor();
    ~Actor();
    
    virtual void onMatrixChanged ();
    virtual void mulMatrixLeft (const Matrix4x4 &m);
    void mulMatrixRight (const Matrix4x4 &m);
    void translate (Float x, Float y, Float z);
    //void setMatrix(const Matrix4x4 &m);
    //void mulMatrixLeft(const Matrix4x4 &m);
    //void mulMatrixRight(const Matrix4x4 &m);
    const Matrix4x4& getMatrix () { return actor2world; }
    
    void setMass (Float mass);
    Float getMass ();
    
    //- translation speed
    //- rotation speed
    //
    //- forces (list of all the forces affecting the object)
    //- totalForce (changes acc. / dir. of speed over time)
    //- addForce(Vector)
    //- removeForce(Force *f)
    //- push (changes acc. / dir. of speed & rotation instantly)
    //
    //- isInCollision()
    //- unCollide(SLIDE | REVERT)

    void setMaterial(Material *material);
    Material* getMaterial();
    
    //Need signed int here so we can pass in -1 as any
    virtual void render (MaterialID materialID) {}
  };
  
  /*
  --------------------------------------------------
  Group object allows for building of hierarchical
  scene tree. Its transformation affects all the
  children objects.
  --------------------------------------------------*/

  class GE_API_ENTRY Group : public Actor
  {
    DECLARE_SUBCLASS (Group, Actor); DECLARE_END;

  private:
    ArrayListT<Actor*> children;

  public:
    void addChild (Actor* o);
    void removeChild (Actor* o);
    const ArrayListT<Actor*>* getChildren ();

    virtual void render (MaterialID materialID);
  };


}//namespace GE
#endif//__GEACTOR_H
