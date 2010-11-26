#ifndef __GESHAPE_H
#define __GESHAPE_H

#include "util/geUtil.h"
#include "engine/geActor.h"
#include "engine/geMaterial.h"

namespace GE
{
  /*----------------------------------------------
   * Forward declarations
   *----------------------------------------------*/
  class PolyMesh;
  class TexMesh;
  class Renderer;
  class SaverObj;

  /*----------------------------------------------
   * A renderable object. Its geometry is defined
   * with either a static or animated mesh. The
   * material defines how geometry is finally
   * rendered.
   *----------------------------------------------*/

  class PolyMeshActor : public Actor3D
  {
    CLASS( PolyMeshActor, Actor3D,
      98e31739,9aaa,4501,877247ba202df017 );

  protected:
    friend class Renderer;
    friend class SaverObj;

    TexMesh *texMesh;
    PolyMesh *polyMesh;
    
    virtual void renderMesh (MaterialID materialID);
    
  public:
    PolyMeshActor();
    ~PolyMeshActor();

    void setMesh (PolyMesh *mesh);
    void setTexMesh (TexMesh *mesh);

    PolyMesh* getMesh();
    TexMesh* getTexMesh ();
    
    virtual void render (MaterialID materialID);
  };
}

#endif /* __GESHAPE_H */
