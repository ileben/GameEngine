#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Actor);
  DEFINE_CLASS (Group);

  Actor::Actor()
  {
    parent = NULL;
    material = NULL;
  }

  Actor::~Actor ()
  {
    if (material != NULL)
      material->dereference();
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

  void Actor::setMaterial(Material *mat)
  {
    if (material != NULL)
      material->dereference();

    material = mat;
    material->reference();
  }
  
  Material* Actor::getMaterial() {
    return material;
  }
  
  void Group::addChild (Actor* o)
  {
    if (o->parent != NULL)
      o->parent->removeChild( o );
    
    children.pushBack( o );
    o->parent = this;
  }
  
  void Group::removeChild (Actor* o)
  {
    children.remove( o );
    o->parent = NULL;
  }
  
  const OCC::ArrayList<Actor*>* Group::getChildren() {
    return &children;
  }

  void Group::render (MaterialID materialID)
  {
    //Hrmm.... don't use this yet... needs some thought
    for (int c=0; c<children.size(); ++c)
      children[ c ]->render( materialID );
  }

}//namespace GE
