#ifndef __UIACTOR_H
#define __UIACTOR_H

#include "util/geUtil.h"
#include "math/geMath.h"

namespace GE
{
  /*
  ----------------------------------------------
  Forward declarations
  ----------------------------------------------*/
  class Actor;
  class Scene;
  class Stage;


  /*
  ----------------------------------------------
  Pin
  ----------------------------------------------*/

  class Pin
  {
  private:
    Uint id;
    ArrayList <Actor*> targets; 

    virtual void otherPin (Actor *w) {};
    virtual void thisPin (Actor *w) {};
    virtual void bothPins (Actor *w) {};

  public:
    void drop (Actor *w);
    void lift ();
    
    Actor* intersect (const Pin &p);
  };

  /*
  ----------------------------------------------
  Event
  ----------------------------------------------*/

  class Event : public Object
  {
    DECLARE_SUBCLASS( Event, Object );
    DECLARE_END;

  private:
    Actor *target;

  public:
    void trigger (Actor *w, bool bubbleUp=false);
  };

  /*
  ----------------------------------------------
  Stage
  ----------------------------------------------*/

  class Stage
  {
    friend class Actor;

  private:
    static Stage *instance;

  public:
    static Stage* GetInstance() { return instance; }

  private:
    ArrayList <Actor*> invalidActors;
  
  public:
    Stage();
    void clearInvalid ();
  };

  /*
  ----------------------------------------------
  Actor
  ----------------------------------------------*/

  class Actor : public Object
  {
    friend class Scene;
    DECLARE_SERIAL_SUBCLASS( Actor, Object );
    DECLARE_OBJREF( scene );
    DECLARE_OBJREF( parent );
    DECLARE_OBJVAR( children );
    DECLARE_DATAVAR( mat );
    DECLARE_END;

  private:
    bool valid;
    Scene *scene;
    Actor *parent;

  protected:
    Vector2 loc;
    Vector2 box;
    Matrix4x4 mat;
    ObjPtrArrayList <Actor> children;

  public:
    Actor (SM *sm); 
    Actor ();
    virtual ~Actor() {}

    void destroy ();
    bool isValid () { return valid; }

    //Location
    Matrix4x4& getMatrix() { return mat; }
    virtual Matrix4x4 getGlobalMatrix (bool inclusive = true);

    void setLoc (float x, float y);
    void setLoc (const Vector2 &loc);
    Vector2& getLoc () { return loc; }
    
    virtual void setSize (float w, float h);
    virtual void setSize (const Vector2 &s);
    virtual Vector2 getSize () { return box; }

    inline Float getLeft ();
    inline Float getRight ();
    inline Float getTop ();
    inline Float getBottom ();
    inline Vector2 getTopLeft ();
    inline Vector2 getTopRight ();
    inline Vector2 getBottomLeft ();
    inline Vector2 getBottomRight ();

    virtual bool hitTest (float x, float y);

    //Structure
    void addChild (Actor* c);
    void removeChild (Actor* c);
    void setParent (Actor* c);
    void getAncestors (ArrayList<Actor*> &list);
    const ObjPtrArrayList<Actor> & getChildren () { return children; }

    bool isRoot () { return scene != NULL; }
    Actor* getParent () { return parent; }
    Scene* getScene();

    //Events
    virtual void onEvent (Event *e) {}
  };

  /*
  ----------------------------------------------
  Scene
  ----------------------------------------------*/

  class Scene : public Object
  {
    DECLARE_SERIAL_SUBCLASS( Scene, Object );
    DECLARE_OBJPTR( root );
    DECLARE_END;

  private:
    bool changed;
    Actor* root;
    ArrayList <Actor*> traversal;

  public:
    Scene ();
    Scene (SM *sm);
    virtual ~Scene() {};

    void setRoot (Actor *actor);
    Actor* getRoot () { return root; }

    void updateChanges ();
    void markChanged ();
    bool hasChanged ();

    Actor* findTopActorAt (float x, float y);
    const ArrayList<Actor*> & getTraversal () { return traversal; }
    
    Actor* findFirstActorByClass (ClassPtr cls);
    void findActorsByClass (ClassPtr cls, ArrayList< Actor* > &outActors);
  };

  /*
  Inlines
  -----------------------------------------------*/

  Float Actor::getLeft ()
    { return loc.x; }
  
  Float Actor::getRight ()
    { return loc.x + box.x; }
  
  Float Actor::getTop ()
    { return loc.y; }
  
  Float Actor::getBottom ()
    { return loc.y + box.y; }

  Vector2 Actor::getTopLeft ()
    { return Vector2( getLeft(), getTop() ); }
  
  Vector2 Actor::getTopRight ()
    { return Vector2( getRight(), getTop() ); }
  
  Vector2 Actor::getBottomLeft ()
    { return Vector2( getLeft(), getBottom() ); }

  Vector2 Actor::getBottomRight ()
    { return Vector2( getRight(), getBottom() ); }

}//namespace GE
#endif//__UIACTOR_H
