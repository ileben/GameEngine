#include "geCamera.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_SERIAL_CLASS (Camera,   ClassID( 0x45278919u, 0x6c8c, 0x4bcc, 0x82471ea1180a6142ull ));
  DEFINE_SERIAL_CLASS (Camera3D, ClassID( 0x8c63ffd7u, 0xdbb4, 0x41a4, 0x870babda1565b1eeull ));
  DEFINE_CLASS (Camera2D);

  Camera::Camera ()
  {
    nearClip = 0.1f;
    farClip = 300.0f;
    setIsRenderable( false );
  }

  Camera::Camera (SM *sm) : Actor3D (sm)
  {
  }

  /*
  Normalize the matrix so the precision errors don't accumulate */

  void Camera::onMatrixChanged ()
  {
    Actor3D::onMatrixChanged ();
    mat.affineNormalize();
  }

  /*
  Camera vectors are the columns of its transform matrix */

  Vector3 Camera::getSide() {
    const Matrix4x4 &m = getMatrix();
    return Vector3( m.m[0][0], m.m[0][1], m.m[0][2] );
  }

  Vector3 Camera::getUp() {
    const Matrix4x4 &m = getMatrix();
    return Vector3( m.m[1][0], m.m[1][1], m.m[1][2] );
  }

  Vector3 Camera::getLook() {
    const Matrix4x4 &m = getMatrix();
    return Vector3( m.m[2][0], m.m[2][1], m.m[2][2] );
  }

  Vector3 Camera::getEye() {
    const Matrix4x4 &m = getMatrix();
    return Vector3( m.m[3][0], m.m[3][1], m.m[3][2] );
  }

  void Camera::setFarClipPlane(Float farClip) {
    this->farClip = farClip;
  }

  Float Camera::getFarClipPlane(){
    return farClip;
  }

  void Camera::setNearClipPlane(Float nearClip) {
    this->nearClip = nearClip;
  }

  Float Camera::getNearClipPlane() {
    return nearClip;
  }

  
  /*======================================
   *
   * Camera3D
   *
   *======================================*/

  Camera3D::Camera3D()
  {
    fov = 45.0f;
  }

  Camera3D::Camera3D (SM *sm) : Camera (sm)
  {
  }

  void Camera3D::setFov (Float fieldOfView) {
    fov = fieldOfView;
  }

  Float Camera3D::getFov () {
    return fov;
  }

  void Camera3D::setCenter (const Vector3 &center) {
    this->center = center;
    cPlus.setTranslation( center.x, center.y, center.z );
    cMinus.setTranslation( -center.x, -center.y, -center.z );
  }

  const Vector3& Camera3D::getCenter () {
    return center;
  }

  void Camera3D::roll (Float radang)
  {
    Matrix4x4 mroll;
    mroll.fromAxisAngle( getLook(), radang );
    mulMatrixLeft( mroll );
  }

  void Camera3D::orbitH (Float radang, bool useCenter)
  {
    Matrix4x4 oH;
    oH.setRotationY( radang );
    
    if (useCenter)
      oH = cPlus * oH * cMinus;
    
    mulMatrixLeft( oH );
  }

  void Camera3D::orbitV (Float radang, bool useCenter)
  {
    Matrix4x4 oV;
    oV.fromAxisAngle( getSide(), radang );
    
    if (useCenter)
      oV = cPlus * oV * cMinus;
    
    mulMatrixLeft( oV );
  }

  void Camera3D::panH (Float dist)
  {
    Matrix4x4 mpan;
    mpan.setTranslation( getSide() * dist );
    mulMatrixLeft( mpan );
  }

  void Camera3D::panV (Float dist)
  {
    Matrix4x4 mpan;
    mpan.setTranslation( getUp() * dist );
    mulMatrixLeft( mpan );
  }

  void Camera3D::zoom (Float dist)
  {
    Matrix4x4 mzoom;
    mzoom.setTranslation( getLook() * dist );
    mulMatrixLeft( mzoom );
  }

  /*
  -----------------------------------------------------------
  A left-handed projection matrix is set up, making the
  camera look down the positive Z axis.

  By default OpenGL uses a constant eye-to-vertex vector
  as if the viewpoint was at an infinite distance. This
  vector is (regardless of the handedness of the projection
  matrix) assumed to point towards negative Z axis. This
  makes specular highlights invisible when the projection
  matrix is left-handed. As a workaround the more realistic
  local-viewer model is enabled.
  -----------------------------------------------------------*/

  void Camera3D::updateProjection(int w, int h)
  {
    Matrix4x4 m;
    m.setPerspectiveFovLH( fov, (float)w/h, nearClip, farClip );

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( (GLfloat*) m.m );
    
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
    glFrontFace( GL_CCW );
  }
  
  void Camera3D::updateView()
  {
    Matrix4x4 world2cam = getGlobalMatrix().affineNormalize().affineInverse();

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
