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
  
  class GE_API_ENTRY TriMeshActor : public Actor
  {
    friend class Renderer;
    friend class SaverObj;
    DECLARE_SUBCLASS (TriMeshActor, Actor);
    DECLARE_END;

  protected:
    TriMesh *mesh;
    ArrayList<Int32> attribIDs;
    
    void beginVertexData (Shader *shader, VFormat *format, void *data);
    void endVertexData (Shader *shader, VFormat *format);
    
  public:
    virtual ClassPtr getShaderComposingClass() { return Class(TriMeshActor); }
    virtual void composeShader( Shader *shader );

    TriMeshActor();
    virtual ~TriMeshActor();

    void setMesh (TriMesh *mesh);
    TriMesh* getMesh();
    
    virtual void render (MaterialID materialID);
  };

  /*
  ===============================================
  Tri mesh with tangent data
  ===============================================*/

  class TanTriMeshTraits
  {
  public:
    struct Vertex
    {
      Vector2 texcoord;
      Vector3 normal;
      Vector3 point;
      Vector3 tangent;
      Vector3 bitangent;
    };

    class VertexFormat : public VFormat { public:
      VertexFormat() : VFormat(sizeof(Vertex))
      {
        addMember( VFMember( ShaderData::TexCoord,  DataUnit::Vec2, sizeof(Vector2) ) );
        addMember( VFMember( ShaderData::Normal,    DataUnit::Vec3, sizeof(Vector3) ) );
        addMember( VFMember( ShaderData::Coord,     DataUnit::Vec3, sizeof(Vector3) ) );
        addMember( VFMember( ShaderData::Attribute, DataUnit::Vec3, sizeof(Vector3),
                             "Tangent", DataUnit::Vec3 ));
        addMember( VFMember( ShaderData::Attribute, DataUnit::Vec3, sizeof(Vector3),
                             "Bitangent", DataUnit::Vec3 ));
      }
    };
  };

  class TanTriMesh : public TriMeshBase <TanTriMeshTraits, TriMesh>
  {
    DECLARE_SERIAL_SUBCLASS( TanTriMesh, TriMesh );
    DECLARE_END;

  public:
    TanTriMesh (SerializeManager *sm) : TriMeshBase <TanTriMeshTraits, TriMesh> (sm) {}
    TanTriMesh () {}
  };

  class TanTriMeshActor : public TriMeshActor
  {
    DECLARE_SUBCLASS (TriMeshActor, Actor);
    DECLARE_END;

  public:
    virtual ClassPtr getShaderComposingClass() { return Class(TanTriMeshActor); }
    virtual void composeShader( Shader *shader ) { TriMeshActor::composeShader( shader ); }
  };


}//namespace GE
#endif//__GETRIMESHACTOR_H
