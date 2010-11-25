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
    CLASS( Camera, 5d6e19f1,1408,4c3d,a5c20914b0da79a4 );
    virtual void serialize( Serializer *s, Uint v)
    {
      Actor3D::serialize( s,v );
      s->data( &farClip );
      s->data( &nearClip );
    }

    friend class Renderer;
    
  protected:
    Float farClip;
    Float nearClip;

    virtual void onMatrixChanged ();
    virtual void updateProjection (Float w, Float h) {}
    virtual void updateView() {}

  public:
    Camera ();
    virtual ~Camera() {};

    Vector3 getEye();
    Vector3 getSide();
    Vector3 getLook();
    Vector3 getUp();

    void setFarClipPlane(Float farClip);
    Float getFarClipPlane();

    void setNearClipPlane(Float nearClip);
    Float getNearClipPlane();

    virtual Matrix4x4 getProjection (Float w, Float h) { return Matrix4x4(); }
  };

  /*------------------------------------------
   * Camera to specify a 3D projection
   *------------------------------------------*/

  struct DofParams
  {
    Float focusCenter;
    Float focusRange;
    Float falloffNear;
    Float falloffFar;
  };

  class Camera3D : public Camera
  {
    CLASS( Camera3D, 73006e7d,53b2,4f15,82ea35735cc834fa );
    virtual void serialize( Serializer *s, Uint v)
    {
      Camera::serialize( s,v );
      s->data( &fov );
      s->data( &dofParams );
      s->data( &dofEnabled );
      s->data( &center );
      s->data( &cPlus );
      s->data( &cMinus );
    }

    friend class Renderer;

  private:
    Float fov;

    Vector3 center;
    Matrix4x4 cPlus;
    Matrix4x4 cMinus;

    DofParams dofParams;
    bool dofEnabled;
    
    virtual void updateProjection (Float w, Float h);
    virtual void updateView();

  public:

    Camera3D ();
    void onDeserialized (void *param);
    
    void setFov (Float fieldOfView);
    Float getFov ();
    
    void setCenter (const Vector3 &center);
    const Vector3& getCenter ();

    void orbitH (Float radang, bool useCenter=false);
    void orbitV (Float radang, bool useCenter=false);
    void roll (Float radang);
    void panH (Float distance);
    void panV (Float distance);
    void zoom (Float distance);

    void setDofParams (Float focusCenter, Float focusRange, Float falloffNear, Float falloffFar);
    void setDofParams (const DofParams &params);
    DofParams getDofParams ();

    void setDofEnabled (bool enabled);
    bool getDofEnabled ();

    virtual Matrix4x4 getProjection (Float w, Float h);
  };

  /*--------------------------------------------
   * Camera for 2D overlays
   *--------------------------------------------*/

  class Camera2D : public Camera
  {
    CLASS( Camera2D, a224e13d,77bd,44bc,92f1177e20c1ac6a );
    friend class Renderer;

  private:
    Vector2 eye;

    virtual void updateProjection (Float w, Float h);
    virtual void updateView();

  public:
    
    void zoom(Float factor);
    void pan(Float x, Float y);
  };
}

#endif /* __GECAMERA_H */
