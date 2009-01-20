#ifndef __GETRIMESHACTOR_H
#define __GETRIMESHACTOR_H

#include "util/geUtil.h"
#include "engine/geActor.h"
#include "engine/geMaterial.h"

namespace GE
{
  /*
  ----------------------------------------------
  Forward declarations
  ----------------------------------------------*/
  class TriMesh;
  class Renderer;
  class SaverObj;

  /*
  ----------------------------------------------
  An actor that draws a TriMesh
  ----------------------------------------------*/
  
  class GE_API_ENTRY TriMeshActor : public Actor
  {
    friend class Renderer;
    friend class SaverObj;
    DECLARE_SUBCLASS (TriMeshActor, Actor);
    DECLARE_END;

  protected:
    TriMesh *mesh;

    virtual void renderMesh (MaterialID materialID);
    
  public:
    TriMeshActor();
    virtual ~TriMeshActor();

    void setMesh (TriMesh *mesh);
    TriMesh* getMesh();
    
    virtual void render (MaterialID materialID);
  };


}//namespace GE
#endif//__GETRIMESHACTOR_H
