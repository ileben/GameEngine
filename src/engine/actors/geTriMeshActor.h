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
    CLASS( TriMeshActor, Actor3D,
      726c0f2f,b78c,45a9,a4e708961c699d5b );

    virtual void serialize( Serializer *s, Uint v )
    {
      Actor3D::serialize( s,v );
      s->object( &mesh );
    }

  protected:
    
    friend class Renderer;
    friend class SaverObj;

    MeshRef mesh;
    
    Uint meshVAO;
    bool meshVAOInit;
    
    virtual void bindBuffers();
    virtual void bindFormat (Shader *shader, VertexFormat *format);
    virtual void renderGroup (const TriMesh::IndexGroup &grp);
    virtual void unbindFormat (Shader *shader, VertexFormat *format);
    virtual void unbindBuffers();

    void renderSingleMat ();
    void renderMultiMat ();
    void renderShadowSingle ();
    void renderShadowMulti ();
    
  public:

    virtual Class getShaderComposingClass() { return ClassName( TriMeshActor ); }
    virtual void composeShader( Shader *shader );

    TriMeshActor ();
    virtual ~TriMeshActor();

    void setMesh (TriMesh *mesh);
    void setMesh (const CharString &name);
    TriMesh* getMesh();

    virtual BoundingBox getBoundingBox();
    virtual void render (RenderTarget::Enum target);
  };


}//namespace GE
#endif//__GETRIMESHACTOR_H
