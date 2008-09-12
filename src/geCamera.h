#ifndef __GECAMERA_H
#define __GECAMERA_H

namespace GE
{
  /*------------------------------------------
   * Forward declarations
   *------------------------------------------*/
  class Renderer;

  /*------------------------------------------
   *  Point-of-view for rendering
   *------------------------------------------*/

  class GE_API_ENTRY Camera : public Actor
  {
    DECLARE_SUBABSTRACT (Camera, Actor); DECLARE_END;
    friend class Renderer;

  private:
    virtual void updateProjection(int w, int h) = 0;
    virtual void updateView() = 0;
  };

  /*------------------------------------------
   * Camera to specify a 3D projection
   *------------------------------------------*/

  class GE_API_ENTRY Camera3D : public Camera
  {
    DECLARE_SUBCLASS (Camera3D, Camera); DECLARE_END;
    friend class Renderer;

  private:
    Vector3 eye;
    Vector3 look;
    Vector3 side;
    Vector3 up;
    Float fov;
    Float farClip;
    Float nearClip;

    Vector3 center;
    Matrix4x4 cPlus;
    Matrix4x4 cMinus;

    void updateProjection(int w, int h);
    void updateView();

  public:
    Camera3D();
    
    virtual void onMatrixChanged ();
    
    void setFov(Float fieldOfView);
    Float getFov();

    void setFarClipPlane(Float farClip);
    Float getFarClipPlane();

    void setNearClipPlane(Float nearClip);
    Float getNearClipPlane();
    
    void setCenter(const Vector3 &center);
    
    const Vector3& getEye();
    const Vector3& getCenter();
    const Vector3& getSide();
    const Vector3& getLook();
    const Vector3& getUp();

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

  class GE_API_ENTRY Camera2D : public Camera
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
