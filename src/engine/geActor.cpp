#include "geActor.h"
#include "geScene.h"
#include "geGLHeaders.h"

namespace GE
{
  Actor3D::Actor3D ()
  {
    material = NULL;
    renderable = true;
    castShadow = true;
    maxDrawDistance = -1.0f;
  }

  Actor3D::~Actor3D ()
  {
    if (material != NULL)
      material->dereference();
  }

  void Actor3D::setMaxDrawDistance (Float d) {
    maxDrawDistance = d;
  }

  Float Actor3D::getMaxDrawDistance () {
    return maxDrawDistance;
  }

  void Actor3D::setCastShadow (bool cast) {
    castShadow = cast;
  }

  bool Actor3D::getCastShadow () {
    return castShadow;
  }
  
  void Actor3D::onMatrixChanged ()
  {
  }
  
  void Actor3D::mulMatrixLeft (const Matrix4x4 &m)
  {
    mat = m * mat;
    onMatrixChanged();
  }
  
  void Actor3D::mulMatrixRight (const Matrix4x4 &m)
  {
    mat *= m;
    onMatrixChanged();
  }

  void Actor3D::setMatrix (const Matrix4x4 &m)
  {
    mat = m;
    onMatrixChanged();
  }

  void Actor3D::lookAt (const Vector3 &look, Vector3 up)
  {
    //Avoid up vector that matches the look vector
    if (Vector::Cross( up, look ).norm() == 0.0 )
      up.set( up.z, up.x, up.y );

    //Assume look is to be the zAxis in world space
    Vector3 xAxis = Vector::Cross( up, look ).normalize();
    Vector3 yAxis = Vector::Cross( look, xAxis ).normalize();
    mat.setColumn( 0, xAxis.xyz(0.0f) );
    mat.setColumn( 1, yAxis.xyz(0.0f) );
    mat.setColumn( 2, look.xyz(0.0f) );
    mat.affineNormalize();
    onMatrixChanged();
  }

  void Actor3D::lookInto (const Vector3 &point, Vector3 up)
  {
    Vector3 center = mat.getColumn(3).xyz();
    lookAt( point - center, up );
  }
  
  void Actor3D::translate (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setTranslation( x,y,z );
    mulMatrixLeft( m );
  }

  void Actor3D::translate (const Vector3 &t) {
    translate( t.x, t.y, t.z );
  }

  void Actor3D::scale (Float x, Float y, Float z)
  {
    Matrix4x4 m;
    m.setScale( x,y,z );
    mulMatrixLeft( m );
  }

  void Actor3D::scale (Float k)
  {
    Matrix4x4 m;
    m.setScale( k );
    mulMatrixLeft( m );
  }

  void Actor3D::rotate (const Vector3 &axis, Float angle)
  {
    Matrix4x4 m;
    m.fromAxisAngle( axis, angle );
    mulMatrixLeft( m );
  }

  void Actor3D::setMaterial (Material *mat)
  {
    if (material != NULL)
      material->dereference();

    material = mat;
    material->reference();
  }
  
  Material* Actor3D::getMaterial() {
    return material;
  }
  
  void Actor3D::setIsRenderable (bool r)
  {
    renderable = r;
  }

  bool Actor3D::isRenderable ()
  {
    return renderable;
  }

}//namespace GE
