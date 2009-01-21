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
    Actor *actor;
    TravEvent::Enum event;
    TravNode () {}
    TravNode( Actor *actor, TravEvent::Enum event )
      { this->actor = actor; this->event = event; }
  };

  /*
  ----------------------------------------------
  A renderable scene - root for object tree
  ----------------------------------------------*/

  class GE_API_ENTRY Scene : public Actor
  {
    DECLARE_SUBCLASS (Scene, Actor); DECLARE_END;
    friend class Renderer;

  private:
    bool changed;
    Camera *cam;
    ArrayList< Light* > lights;
    ArrayList< TravNode > traversal;

  protected:
    bool hasChanged();
    void updateChanges();

  public:

    Scene();
    void markChanged();
    void bindCamera( Camera *cam );
    inline const ArrayList< TravNode >* getTraversal();
    inline const ArrayList< Light* >* getLights();
  };

  const ArrayList< TravNode >* Scene::getTraversal() {
    return &traversal;
  }

  const ArrayList< Light* >* Scene::getLights() {
    return &lights;
  }
}

#endif /* __GESCENE_H */
