#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (Camera);
  DEFINE_CLASS (Camera3D);
  DEFINE_CLASS (Camera2D);
  
  /*======================================
   *
   * Camera3D
   *
   *======================================*/

  Camera3D::Camera3D()
  {
    eye .set (0,0,0);
    side.set (1,0,0);
    up  .set (0,1,0);
    look.set (0,0,1);
    
    fov = 45.0f;
    nearClip = 0.1f;
    farClip = 300.0f;
  }

  void Camera3D::setFov(Float fieldOfView) {
    fov = fieldOfView;
  }

  Float Camera3D::getFov() {
    return fov;
  }

  void Camera3D::setFarClipPlane(Float farClip) {
    this->farClip = farClip;
  }

  Float Camera3D::getFarClipPlane(){
    return farClip;
  }

  void Camera3D::setNearClipPlane(Float nearClip) {
    this->nearClip = nearClip;
  }

  Float Camera3D::getNearClipPlane() {
    return nearClip;
  }

  const Vector3& Camera3D::getEye() {
    return eye;
  }

  void Camera3D::setCenter(const Vector3 &center) {
    this->center = center;
    cPlus.setTranslation (center.x, center.y, center.z);
    cMinus.setTranslation (-center.x, -center.y, -center.z);
  }

  const Vector3& Camera3D::getCenter() {
    return center;
  }

  const Vector3& Camera3D::getSide() {
    return side;
  }

  const Vector3& Camera3D::getLook() {
    return look;
  }

  const Vector3& Camera3D::getUp() {
    return up;
  }
  
  void Camera3D::onMatrixChanged ()
  {
    Actor::onMatrixChanged ();
    
    //Normalize the matrix so the precision errors don't accumulate
    actor2world.affineNormalize();
    
    //Camera vectors are first three columns of its transform matrix
    const Matrix4x4 &cam2world = getMatrix();
    side.set ( cam2world.m [0][0],  cam2world.m [0][1],  cam2world.m [0][2]);
    up  .set ( cam2world.m [1][0],  cam2world.m [1][1],  cam2world.m [1][2]);
    look.set ( cam2world.m [2][0],  cam2world.m [2][1],  cam2world.m [2][2]);
    eye .set ( cam2world.m [3][0],  cam2world.m [3][1],  cam2world.m [3][2]);
  }

  void Camera3D::roll(Float radang)
  {
    Matrix4x4 mroll;
    mroll.fromAxisAngle (look, radang);
    mulMatrixLeft (mroll);
  }

  void Camera3D::orbitH(Float radang, bool useCenter)
  {
    Matrix4x4 oH;
    oH.setRotationY (radang);
    
    if (useCenter)
      oH = cPlus * oH * cMinus;
    
    mulMatrixLeft (oH);
  }

  void Camera3D::orbitV(Float radang, bool useCenter)
  {
    Matrix4x4 oV;
    oV.fromAxisAngle (side, radang);
    
    if (useCenter)
      oV = cPlus * oV * cMinus;
    
    mulMatrixLeft (oV);
  }

  void Camera3D::panH(Float dist)
  {
    Matrix4x4 mpan;
    mpan.setTranslation (side * dist);
    mulMatrixLeft (mpan);
  }

  void Camera3D::panV(Float dist)
  {
    Matrix4x4 mpan;
    mpan.setTranslation (up * dist);
    mulMatrixLeft (mpan);
  }

  void Camera3D::zoom(Float dist)
  {
    Matrix4x4 mzoom;
    mzoom.setTranslation (look * dist);
    mulMatrixLeft (mzoom);
  }
  
  void Camera3D::updateProjection(int w, int h)
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(fov, (GLfloat)w/h, nearClip, farClip);
    Matrix4x4 m;
    m.setPerspectiveFovLH( fov, (float)w/h, nearClip, farClip );
    glLoadMatrixf( (GLfloat*) m.m );
    
    glFrontFace( GL_CW );
  }
  
  void Camera3D::updateView()
  {
    Matrix4x4 world2cam = getMatrix().affineInverse();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf ((GLfloat*) world2cam.m);
  }

  /*======================================
   *
   * Camera2D
   *
   *======================================*/

  void Camera2D::updateProjection(int w, int h)
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
  }

  void Camera2D::updateView()
  {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //TODO: add transformation according to zoom and pan
  }

}/* namespace GE */
