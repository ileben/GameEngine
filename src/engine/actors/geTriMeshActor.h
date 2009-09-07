#ifndef __GETRIMESHACTOR_H
#define __GETRIMESHACTOR_H

#include "util/geUtil.h"
#include "engine/geActor.h"
#include "engine/geMaterial.h"
#include "engine/geTriMesh.h"

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
  
  class TriMeshActor : public Actor3D
  {
    DECLARE_SERIAL_SUBCLASS( TriMeshActor, Actor3D );
    DECLARE_OBJVAR( mesh );
    DECLARE_END;

    friend class Renderer;
    friend class SaverObj;

  protected:
    MeshRef mesh;
    ArrayList<Int32> attributeIDs;
    
    void beginVertexData (Shader *shader, VertexFormat *format, void *data);
    void endVertexData (Shader *shader, VertexFormat *format);
    
  public:
    virtual ClassPtr getShaderComposingClass() { return Class(TriMeshActor); }
    virtual void composeShader( Shader *shader );

    TriMeshActor (SM *sm) : Actor3D(sm), mesh(sm) {}
    TriMeshActor ();
    virtual ~TriMeshActor();

    void setMesh (TriMesh *mesh);
    void setMesh (const CharString &name);
    TriMesh* getMesh();
    
    virtual void render (MaterialID materialID);
  };


}//namespace GE
#endif//__GETRIMESHACTOR_H
