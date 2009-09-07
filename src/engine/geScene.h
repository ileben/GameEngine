#ifndef __GESCENE_H
#define __GESCENE_H

#include "util/geUtil.h"
#include "geActor.h"

namespace GE
{
  /*
  --------------------------------------------
  Forward declarations
  --------------------------------------------*/

  class Camera;
  class Renderer;
  class Light;

  /*
  --------------------------------------------
  Depth-first traversal data
  --------------------------------------------*/
  
  namespace TravEvent {
    enum Enum {
      Begin,
      End
    };}
  
  class TravNode
  {public:
    Actor3D *actor;
    TravEvent::Enum event;
    TravNode () {}
    TravNode( Actor3D *actor, TravEvent::Enum event )
      { this->actor = actor; this->event = event; }
  };

  /*
  ----------------------------------------------
  A renderable scene - root for object tree
  ----------------------------------------------*/

  class Scene3D : public Scene
  {
    DECLARE_SUBCLASS (Scene3D, Scene); DECLARE_END;
    friend class Renderer;

  private:
    Camera *cam;
    ArrayList< Light* > lights;
    ArrayList< TravNode > traversal;
    Vector3 ambientColor;

  public:
    Scene3D();

    void bindCamera( Camera *cam );
    inline const ArrayList< TravNode >* getTraversal();
    inline const ArrayList< Light* >* getLights();

    void setAmbientColor (const Vector3 &color);
    const Vector3& getAmbientColor ();

    virtual void updateChanges();
  };

  const ArrayList< TravNode >* Scene3D::getTraversal() {
    return &traversal;
  }

  const ArrayList< Light* >* Scene3D::getLights() {
    return &lights;
  }
}

#endif /* __GESCENE_H */
