#include "ui/uiActor.h"

namespace GE
{
  #define CLSID_ACTOR ClassID (0x0acdcd40u, 0x9da9, 0x4dac, 0xac7bccbada6e4ee5ull)
  DEFINE_SERIAL_CLASS( Actor, CLSID_ACTOR  );
  DEFINE_CLASS( Scene );
  DEFINE_CLASS( Event );

  Stage* Stage::instance = NULL;

  Stage::Stage()
  {
    assert( Stage::instance == NULL );
    Stage::instance = this;
  }

  Actor::Actor (SM *sm) : children (sm)
  {
    valid = true;
    parent = NULL;
    scene = NULL;
  }

  Actor::Actor()
  {
    valid = true;
    parent = NULL;
    scene = NULL;
  }

  void Actor::destroy()
  {
    for (UintSize c=0; c<children.size(); ++c)
      children[c]->destroy();

    setParent( NULL );
    valid = false;

    Stage *stage = Stage::GetInstance();
    stage->invalidActors.pushBack( this );
  }

  Scene* Actor::getScene()
  {
    Actor *parent = this;
    while (parent != NULL)
    {
      if (parent->scene != NULL)
        return parent->scene;
      
      parent = parent->getParent();
    }
    
    return NULL;
  }

  void Actor::setParent (Actor* p)
  {
    if (scene != NULL) {
      scene->setRoot( NULL );
      scene = NULL;
    }

    if (parent != NULL) {
      parent->children.remove( this );
      parent = NULL;
    }

    if (p != NULL) {
      p->children.pushBack( this );
      parent = p;
    }

    Scene *w = getScene();
    if (w != NULL) w->markChanged();
  }

  void Actor::addChild (Actor* c)
  {
    if (c->scene != NULL) {
      c->scene->setRoot( NULL );
      c->scene = NULL;
    }

    if (c->parent != NULL) {
      c->parent->children.remove( this );
      c->parent = NULL;
    }
    
    children.pushBack( c );
    c->parent = this;

    Scene *w = getScene();
    if (w != NULL) w->markChanged();
  }
  
  void Actor::removeChild (Actor* c)
  {
    children.remove( c );
    c->parent = NULL;

    Scene *w = getScene();
    if (w != NULL) w->markChanged();
  }

  void Actor::getAncestors (ArrayList<Actor*> &list)
  {
    Actor *parent = getParent();
    while (parent != NULL)
    {
      list.pushBack( parent );
      parent = parent->getParent();
    }
  }

  Scene::Scene()
  {
    changed = false;
    root = NULL;
  }

  void Scene::markChanged()
  {
    changed = true;
  }

  bool Scene::hasChanged ()
  {
    return changed;
  }

  void Scene::updateChanges()
  {
    //Check if changed at all
    if (!changed) return;
    changed = false;

    //Clear old data
    traversal.clear();
    if (root == NULL) return;

    //Push root onto stack
    ArrayList <Actor*> stack;
    stack.pushBack( root );
    while (!stack.empty())
    {
      //Pop top
      Actor* parent = stack.last();
      stack.popBack();

      //Add to traversal
      traversal.pushBack( parent );

      //Push children onto stack
      const ObjPtrArrayList <Actor> & children = parent->getChildren();
      for (UintSize c=0; c<children.size(); ++c)
        stack.pushBack( children[c] );
    }
  }

  void Scene::setRoot (Actor *actor)
  {
    if (root != NULL)
      root->scene = NULL;

    root = actor;
    root->scene = this;
    markChanged();
  }

  void Pin::drop (Actor *w)
  {
    targets.clear();
    if (w != NULL)
    {
      targets.pushBack( w );
      w->getAncestors( targets );
    }
  }

  void Pin::lift ()
  {
    targets.clear();
  }

  Actor* Pin::intersect (const Pin &p)
  {
    Actor *top = NULL;
    Int i = (Int) targets.size()-1;
    Int pi = (Int) p.targets.size()-1;

    //Walk both target lists parent-to-child
    for (; i>=0 && pi>=0; --i, --pi)
    {
      //Stop when the targets don't match
      if (targets[i] != p.targets[pi])
        break;

      //Remember last common target
      top = targets[i];
    }

    //Walk other targets child-to-parent until first common
    for (Int pj=0; pj<=pi; ++pj )
      if (p.targets[pj]->isValid())
        otherPin( p.targets[pj] );

    //Walk these targets child-to-parent until first common
    for (Int j=0; j<=i; ++j)
      if (targets[j]->isValid())
        thisPin( targets[j] );

    //Walk the rest of the targets child-to-parent
    for (Int b=i+1; b<(Int)targets.size(); ++b)
      bothPins( targets[b] );

    //Return highest common target
    return top;
  }

  void Event::trigger (Actor *w, bool bubbleUp)
  {
    //Trigget event on top widget
    this->target = w;
    w->onEvent( this );

    if (bubbleUp)
    {
      //Get all parents
      ArrayList<Actor*> ancestors;
      w->getAncestors( ancestors );

      //Triget event on ancestors
      for (UintSize a=0; a<ancestors.size(); ++a)
        ancestors[a]->onEvent( this );
    }
  }


  //--------------------------------------------------------------

  void Actor::setLoc (float x, float y) {
    loc.set( x, y );
  }

  void Actor::setLoc (const Vector2 &l) {
    setLoc( l.x, l.y );
  }
  
  void Actor::setSize (float w, float h) {
    box.set( w, h );
  }

  void Actor::setSize (const Vector2 &s) {
    setSize( s.x, s.y );
  }

  Matrix4x4 Actor::getGlobalMatrix (bool inclusive)
  {
    Matrix4x4 g;
    if (inclusive)
      g = getMatrix();
    
    Actor *p = getParent();
    while (p != NULL)
    {
      g = p->getMatrix() * g;
      p = p->getParent();
    }

    return g;
  }

  bool Actor::hitTest (float x, float y)
  {
    Matrix4x4 invg = getGlobalMatrix().affineInverse();
    Vector3 hit = invg * Vector3( x,y,0 );
    if (hit.x < loc.x) return false;
    if (hit.y < loc.y) return false;
    if (hit.x > loc.x + box.x) return false;
    if (hit.y > loc.y + box.y) return false;
    return true;
  }

  Actor* Scene::findTopActorAt (float x, float y)
  {
    //Traverse backwards
    for (int t=(int)traversal.size()-1; t>=0; --t) {
      Actor* w = traversal[t];

      //Check for hit
      if (w->hitTest(x, y))
        return w;
    }

    return NULL;
  }

  Actor* Scene::findFirstActorByClass (ClassPtr cls)
  {
    for (UintSize t=0; t<traversal.size(); ++t) {
      Actor* a = (Actor*) SafeCastPtr( cls, traversal[t] );
      if (a != NULL) return a;
    }

    return NULL;
  }

  void Scene::findActorsByClass (ClassPtr cls, ArrayList<Actor*> &outActors)
  {
    for (UintSize t=0; t<traversal.size(); ++t) {
      Actor* a = (Actor*) SafeCastPtr( cls, traversal[t] );
      if (a != NULL) outActors.pushBack( a );
    }
  }
}
