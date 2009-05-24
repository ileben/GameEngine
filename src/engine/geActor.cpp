#include "geActor.h"
#include "geScene.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (Actor);

  Actor::Actor()
  {
    parent = NULL;
    material = NULL;
    renderable = true;
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

  void Actor::lookAt (const Vector3 &look, Vector3 up)
  {
    //Avoid up vector that matches the look vector
    if (Vector::Cross( up, look ).norm() == 0.0 ) {
      up.x += 0.0001f;
      up.y += 0.0002f;
      up.z += 0.0003f;
    }

    //Assume look is to be the zAxis in world space
    Vector3 xAxis = Vector::Cross( up, look ).normalize();
    Vector3 yAxis = Vector::Cross( look, xAxis ).normalize();
    actor2world.setColumn( 0, xAxis.xyz(0.0f) );
    actor2world.setColumn( 1, yAxis.xyz(0.0f) );
    actor2world.setColumn( 2, look.xyz(0.0f) );
    actor2world.affineNormalize();
    onMatrixChanged();
  }

  void Actor::lookInto (const Vector3 &point, Vector3 up)
  {
    Vector3 center = actor2world.getColumn(3).xyz();
    lookAt( point - center, up );
  }

  Matrix4x4 Actor::getWorldMatrix ()
  {
    Actor *p = parent;
    Matrix4x4 world = getMatrix();
    
    while (p != NULL)
    {
      world = p->getMatrix() * world;
      p = p->getParent();
    }
    
    return world;
  }
  
  void Actor::translate (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setTranslation( x,y,z );
    mulMatrixLeft( m );
  }

  void Actor::translate (const Vector3 &t) {
    translate( t.x, t.y, t.z );
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
  
  void Actor::addChild (Actor* o)
  {
    if (o->parent != NULL)
      o->parent->removeChild( o );
    
    children.pushBack( o );
    o->parent = this;

    Scene *scene = getScene();
    if (scene != NULL) scene->markChanged();
  }
  
  void Actor::removeChild (Actor* o)
  {
    children.remove( o );
    o->parent = NULL;

    Scene *scene = getScene();
    if (scene != NULL) scene->markChanged();
  }
  
  const ArrayList<Actor*>* Actor::getChildren() {
    return &children;
  }

  Actor* Actor::getParent()
  {
    return parent;
  }

  Scene* Actor::getScene()
  {
    Actor *a = this;

    while (a != NULL)
    {
      Scene *s = SafeCast( Scene, a );
      if (s != NULL) return s;
      a = a->getParent();
    }

    return NULL;
  }

  void Actor::setIsRenderable (bool r)
  {
    renderable = r;
  }

  bool Actor::isRenderable ()
  {
    return renderable;
  }

  void Actor::begin ()
  {
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( (GLfloat*) getMatrix().m );
  }
  
  void Actor::end ()
  {
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
  }

}//namespace GE
