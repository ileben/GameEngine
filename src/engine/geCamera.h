#ifndef __GECAMERA_H
#define __GECAMERA_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"
#include "geActor.h"

namespace GE
{
  /*------------------------------------------
   * Forward declarations
   *------------------------------------------*/
  class Renderer;

  /*------------------------------------------
   *  Point-of-view for rendering
   *------------------------------------------*/

  class Camera : public Actor3D
  {
    DECLARE_SERIAL_SUBCLASS( Camera, Actor3D );
    DECLARE_DATAVAR( farClip );
    DECLARE_DATAVAR( nearClip );
    DECLARE_END;
    friend class Renderer;
    
  protected:
    Float farClip;
    Float nearClip;

    virtual void onMatrixChanged ();
    virtual void updateProjection(int w, int h) {}
    virtual void updateView() {}

  public:
    Camera ();
    Camera (SM *sm);
    virtual ~Camera() {};

    Vector3 getEye();
    Vector3 getSide();
    Vector3 getLook();
    Vector3 getUp();

    void setFarClipPlane(Float farClip);
    Float getFarClipPlane();

    void setNearClipPlane(Float nearClip);
    Float getNearClipPlane();
  };

  /*------------------------------------------
   * Camera to specify a 3D projection
   *------------------------------------------*/

  class Camera3D : public Camera
  {
    DECLARE_SERIAL_SUBCLASS (Camera3D, Camera);
    DECLARE_DATAVAR( fov );
    DECLARE_DATAVAR( center );
    DECLARE_DATAVAR( cPlus );
    DECLARE_DATAVAR( cMinus );
    DECLARE_END;
    friend class Renderer;

  private:
    Float fov;
    Vector3 center;
    Matrix4x4 cPlus;
    Matrix4x4 cMinus;
    
    void updateProjection(int w, int h);
    void updateView();

  public:

    Camera3D ();
    Camera3D (SM *sm);
    void onDeserialized (void *param);
    
    void setFov(Float fieldOfView);
    Float getFov();
    
    void setCenter(const Vector3 &center);
    const Vector3& getCenter();

    void orbitH(Float radang, bool useCenter=false);
    void orbitV(Float radang, bool useCenter=false);
    void roll(Float radang);
    void panH(Float distance);
    void panV(Float distance);
    void zoom(Float distance);
  };

  /*--------------------------------------------
   * Camera for 2D overlays
   *--------------------------------------------*/

  class Camera2D : public Camera
  {
    DECLARE_SUBCLASS (Camera2D, Camera); DECLARE_END;
    friend class Renderer;

  private:
    Vector2 eye;

    void updateProjection(int w, int h);
    void updateView();

  public:
    
    void zoom(Float factor);
    void pan(Float x, Float y);
  };
}

#endif /* __GECAMERA_H */
