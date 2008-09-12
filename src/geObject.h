#ifndef __GEOBJECT_H
#define __GEOBJECT_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*---------------------------------------------
   * Forward declarations
   *---------------------------------------------*/
  class Group;

  
  /*----------------------------------------------
   * Object class can be automagically saved to
   * or loaded from either text or binary format
   * saving or loading all of its properties
   *----------------------------------------------*/
  
  class GE_API_ENTRY Object
  {
    DECLARE_CLASS (Object); DECLARE_END;
    
  protected:
    
    OCC::String id;
    
  public:
    
    void setId (const OCC::String &id);
    const OCC::String& getId ();
  };
  
  /*-------------------------------------------------
   * Actor represents a single node to be placed
   * in the scene tree. It can be a shape, camera
   * or any other kind of actor (derived from the
   * base). Transformation matrix allows for
   * additional transformation of the object content
   *-------------------------------------------------*/
  
  class GE_API_ENTRY Actor : public Object
  {
    DECLARE_SUBCLASS (Actor, Object); DECLARE_END;
    friend class Group;
    
  private:
    
    Matrix4x4 actor2world;
    
  protected:
    
    Group *parent;
    Float mass;
    
  public:
    
    Actor();
    
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
  };
  
  /*--------------------------------------------------
   * Group object allows for building of hierarchical
   * scene tree. Its transformation affects all the
   * children objects.
   *--------------------------------------------------*/


  class GE_API_ENTRY Group : public Actor
  {
    DECLARE_SUBCLASS (Group, Actor); DECLARE_END;

  private:
    OCC::ArrayList<Actor*> children;

  public:
    void addChild (Actor* o);
    void removeChild (Actor* o);
    const OCC::ArrayList<Actor*>* getChildren ();
  };

}//namespace GE
#pragma warning(pop)
#endif //__GEOBJECT_H
