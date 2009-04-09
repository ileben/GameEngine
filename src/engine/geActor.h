#ifndef __GEACTOR_H
#define __GEACTOR_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geMatrix.h"
#include "geObject.h"
#include "geMaterial.h"

namespace GE
{
  /*
  -----------------------------------------
  Forward declarations
  -----------------------------------------*/

  class Material;
  class Scene;
  class Kernel;
  class Renderer;

  /*
  -------------------------------------------------
  Actor represents a single node to be placed
  in the scene tree. It can be a shape, camera
  or any other kind of actor (derived from the
  base). Transformation matrix allows for
  additional transformation of the object content
  -------------------------------------------------*/
  
  class GE_API_ENTRY Actor : public Object
  {
    DECLARE_SUBCLASS (Actor, Object); DECLARE_END;
    
    friend class Kernel;
    friend class Renderer;
    
  private:

    Material *material;
    ArrayList<Actor*> children;
    
  protected:

    Matrix4x4 actor2world;
    Actor *parent;
    Float mass;
    bool renderable;

  public:

    Actor();
    virtual ~Actor();
    
    //Transform matrix manipulation
    virtual void onMatrixChanged ();
    void mulMatrixLeft (const Matrix4x4 &m);
    void mulMatrixRight (const Matrix4x4 &m);
    void setMatrix (const Matrix4x4 &m);
    const Matrix4x4& getMatrix () { return actor2world; }
    Matrix4x4 getWorldMatrix ();
    
    //Transform helpers
    void translate (Float x, Float y, Float z);
    void scale (Float x, Float y, Float z);
    void scale (Float k);
    void rotate (const Vector3 &axis, Float angle);
    void lookAt (const Vector3 &look, const Vector3 &up = Vector3(0,1,0));
    void lookInto (const Vector3 &point, const Vector3 &up = Vector3(0,1,0));
    
    //Material assignment
    void setMaterial(Material *material);
    Material* getMaterial();
    
    //Scene tree handling
    void addChild (Actor* o);
    void removeChild (Actor* o);
    const ArrayList<Actor*>* getChildren ();
    Actor* getParent ();
    Scene* getScene();
    void setIsRenderable (bool renderable);
    bool isRenderable ();

    //Shader composing
    virtual ClassPtr getShaderComposingClass() { return Class(Actor); }
    virtual void composeShader( Shader *shader ) {}
    
    //Rendering steps (as invoked by Renderer):
    virtual void prepare () {}
    virtual void begin ();
    //- material begin
    virtual void render (MaterialID materialID) {}
    //- material end
    //- recurse to children
    virtual void end ();
    
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
