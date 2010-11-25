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

  class Event
  {
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

    CLASS( Actor, 0acdcd40,9da9,4dac,ac7bccbada6e4ee5 );
    virtual void serialize (Serializer *s, Uint v)
    {
      Object::serialize( s,v );
      s->string( &name );
      s->objectRef( &scene );
      s->objectRef( &parent );
      s->objectPtrArray( &children );
      s->data( &mat );
    }

  private:
    bool valid;
    Scene *scene;
    Actor *parent;
    CharString name;

  protected:
    Vector2 loc;
    Vector2 box;
    Matrix4x4 mat;
    ArrayList <Actor*> children;

  public:
    Actor ();
    virtual ~Actor() {}

    void destroy ();
    bool isValid () { return valid; }

    void setName (const CharString &name);
    const CharString& getName ();

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
    const ArrayList<Actor*> & getChildren () { return children; }

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
    CLASS( Scene, b70776ed,5881,48fb,8fc46f317579d987 );
    virtual void serialize (Serializer *s, Uint v)
    {
      Object::serialize( s,v );
      s->objectPtr( &root );
    }

  private:
    bool changed;
    Actor* root;
    ArrayList <Actor*> traversal;

  public:
    Scene ();
    virtual ~Scene() {};

    void setRoot (Actor *actor);
    Actor* getRoot () { return root; }

    void updateChanges ();
    void markChanged ();
    bool hasChanged ();

    Actor* findTopActorAt (float x, float y);
    const ArrayList<Actor*> & getTraversal () { return traversal; }
    
    //Actor* findFirstActorByClass (ClassPtr cls);
    //void findActorsByClass (ClassPtr cls, ArrayList< Actor* > &outActors);

    template <class C>
    Actor* findFirstActorByClass ();

    template <class C>
    void findActorsByClass (ArrayList< Actor* > &outActors);

    Actor* findFirstActorByName (const CharString &name);
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


  template <class C>
  Actor* Scene::findFirstActorByClass ()
  {
    for (UintSize t=0; t<traversal.size(); ++t) {
      Actor* a = (Actor*) dynamic_cast< C >( traversal[t] );
      if (a != NULL) return a;
    }

    return NULL;
  }

  template <class C>
  void Scene::findActorsByClass (ArrayList< Actor* > &outActors)
  {
    for (UintSize t=0; t<traversal.size(); ++t) {
      Actor* a = (Actor*) dynamic_cast< C >( traversal[t] );
      if (a != NULL) outActors.pushBack( a );
    }
  }

}//namespace GE
#endif//__UIACTOR_H
