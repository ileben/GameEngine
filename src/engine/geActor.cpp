#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

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
    onMatrixChanged();
  }
  
  void Actor::mulMatrixRight (const Matrix4x4 &m)
  {
    actor2world *= m;
    onMatrixChanged();
  }

  void Actor::setMatrix (const Matrix4x4 &m)
  {
    actor2world = m;
    onMatrixChanged();
  }
  
  void Actor::translate (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setTranslation( x,y,z );
    mulMatrixLeft( m );
  }

  void Actor::scale (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setScale( x,y,z );
    mulMatrixLeft( m );
  }

  void Actor::scale (Float k)
  {
    Matrix4x4 m;
    m.setScale( k );
    mulMatrixLeft( m );
  }

  void Actor::rotate (const Vector3 &axis, Float angle)
  {
    Matrix4x4 m;
    m.fromAxisAngle( axis, angle );
    mulMatrixLeft( m );
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
  
  const ArrayList<Actor*>* Group::getChildren() {
    return &children;
  }

  void Actor::renderBegin()
  {
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( (GLfloat*) getMatrix().m );
  }
  
  void Actor::renderEnd()
  {
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
  }

}//namespace GE
