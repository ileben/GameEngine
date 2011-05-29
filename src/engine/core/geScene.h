#ifndef __GESCENE_H
#define __GESCENE_H

#include "util/geUtil.h"
#include "core/geActor.h"
#include "core/geAnimation.h"

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
    CLASS( Scene3D, Scene,
      935538c5,5cff,4949,86a3fe35a672c019 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Scene::serialize( s,v );
      s->data( &ambientColor );
      s->objectPtrArray( &animations );
      s->objectPtrArray( &resources );
    }

  private:
    friend class Renderer;

    Camera *cam;
    ArrayList< Light* > lights;
    ArrayList< TravNode > traversal;

  private:
    Vector3 ambientColor;

  public:
    ArrayList< Animation* > animations;
    ArrayList< Resource* > resources;

  public:
    Scene3D ();
    ~Scene3D ();

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
