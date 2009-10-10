#ifndef __GEACTOR_H
#define __GEACTOR_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"
#include "ui/uiActor.h"
#include "geMaterial.h"
#include "geRenderer.h"

namespace GE
{
  /*
  -----------------------------------------
  Forward declarations
  -----------------------------------------*/

  class Scene3D;
  class Kernel;
  class Renderer;
  class Material;

  /*
  -------------------------------------------------
  Actor3D represents a single node to be placed
  in the scene tree. It can be a shape, camera
  or any other kind of actor (derived from the
  base). Transformation matrix allows for
  additional transformation of the object content
  -------------------------------------------------*/
  
  class Actor3D : public Actor
  {
    DECLARE_SERIAL_SUBCLASS( Actor3D, Actor );
    DECLARE_DATAVAR( renderable );
    DECLARE_DATAVAR( castShadow );
    DECLARE_DATAVAR( maxDrawDistance );
    DECLARE_OBJPTR( material );
    DECLARE_END;
    
    friend class Kernel;
    friend class Renderer;
    
  private:

    bool renderable;
    bool castShadow;
    Float maxDrawDistance;
    Material *material;

  public:

    Actor3D (SM *sm);
    Actor3D ();
    virtual ~Actor3D ();

    void setMaxDrawDistance (Float d);
    Float getMaxDrawDistance ();
    virtual BoundingBox getBoundingBox() { return BoundingBox(); }

    void setCastShadow (bool cast);
    bool getCastShadow ();

    //Transform matrix manipulation
    virtual void onMatrixChanged ();
    void mulMatrixLeft (const Matrix4x4 &m);
    void mulMatrixRight (const Matrix4x4 &m);
    void setMatrix (const Matrix4x4 &m);
    
    //Transform helpers
    void translate (Float x, Float y, Float z);
    void translate (const Vector3 &t);
    void scale (Float x, Float y, Float z);
    void scale (Float k);
    void rotate (const Vector3 &axis, Float angle);
    void lookAt (const Vector3 &look, Vector3 up = Vector3(0,1,0));
    void lookInto (const Vector3 &point, Vector3 up = Vector3(0,1,0));
    
    //Material assignment
    void setMaterial(Material *material);
    Material* getMaterial();
    
    //Scene tree handling
    void setIsRenderable (bool renderable);
    bool isRenderable ();

    //Shader composing
    virtual ClassPtr getShaderComposingClass() { return Class(Actor3D); }
    virtual void composeShader( Shader *shader ) {}
    
    //Rendering steps (as invoked by Renderer):
    virtual void prepare () {}
    virtual void begin () {}
    virtual void render (RenderTarget::Enum target) {}
    virtual void end () {}

    //Invoked when loading after referenced resources are loaded
    virtual void onResourcesLoaded() {}
    
    //Physics
    //====================================
    //void setMass (Float mass);
    //Float getMass ();
    //
    //- translation speed
    //- rotation speed
    //
    //- forces (list of all the forces affecting the object)
    //- totalForce (changes acc. / dir. of speed over time)
    //- addForce(Vector)
    //- removeForce(Force *f)
    //- push (changes acc. / dir. of speed & rotation instantly)
    //
    //- isInCollision()
    //- unCollide(SLIDE | REVERT)
  };


}//namespace GE
#endif//__GEACTOR_H
