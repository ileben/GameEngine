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

  class GE_API_ENTRY PolyMeshActor : public Actor3D
  {
    friend class Renderer;
    friend class SaverObj;
    DECLARE_SUBCLASS (PolyMeshActor, Actor3D);
    DECLARE_END;

  protected:
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
