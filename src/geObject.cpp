#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Object);
  DEFINE_CLASS (Actor);
  DEFINE_CLASS (Group);
  
  void Object::setId (const String &id)
  {
    this->id = id;
  }
  
  const String& Object::getId ()
  {
    return id;
  }
  
  Actor::Actor()
  {
    parent = NULL;
  }
  
  void Actor::onMatrixChanged ()
  {
  }
  
  void Actor::mulMatrixLeft (const Matrix4x4 &m)
  {
    actor2world = m * actor2world;
    actor2world.affineNormalize ();
    onMatrixChanged ();
  }
  
  void Actor::mulMatrixRight (const Matrix4x4 &m)
  {
    actor2world *= m;
    actor2world.affineNormalize ();
    onMatrixChanged ();
  }
  
  void Actor::translate (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setTranslation (x,y,z);
    mulMatrixLeft (m);
  }
  
  void Group::addChild (Actor* o)
  {
    if (o->parent != NULL)
      o->parent->removeChild(o);
    
    children.pushBack(o);
    o->parent = this;
  }
  
  void Group::removeChild (Actor* o)
  {
    children.remove(o);
    o->parent = NULL;
  }
  
  const OCC::ArrayList<Actor*>* Group::getChildren() {
    return &children;
  }
}
