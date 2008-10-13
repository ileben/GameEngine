#ifndef __GESTATICMESH_H
#define __GESTATICMESH_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*
  ------------------------------------------------------------
  Static mesh is highly optimized for rendering. Note that
  since OpenGL doesn't allow separate indices for vertex
  and texture/color/normal arrays, certain information has
  to be duplicated many times in the respective array in
  case another varies across different faces, adjacent to
  a certain vertex (like normals when flat-shading is used).
  ------------------------------------------------------------*/
  
  typedef Uint32 VertexID;
  
  struct TriMeshVertex
  {
    Vector2 texcoord;
    Vector3 normal;
    Vector3 point;
  };
  
  class GE_API_ENTRY TriMesh : public Resource
  {
    friend class Renderer;
    DECLARE_SERIAL_SUBCLASS( TriMesh, Resource );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    
    struct IndexGroup
    {
      MaterialID materialID;
      VertexID   start;
      VertexID   count;
    };
    
    GenericArrayList data;
    ArrayList <Uint32> indices;
    ArrayList <IndexGroup> groups;
    
  protected:
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::VertexNormal *polyNormal,
                                 TexMesh::Vertex *texVert);
    
    virtual void faceFromPoly (PolyMesh::Face *polyFace);
    
  public:
        
    void serialize (void *sm)
    {
      ( (SM*)sm )->objectVar( &data );
      ( (SM*)sm )->objectVar( &indices );
      ( (SM*)sm )->objectVar( &groups );
    }
    
    TriMesh (SerializeManager *sm) : data(sm), indices(sm), groups(sm)
    {}
    
    TriMesh (Uint32 vertexSize) : data (vertexSize, NULL)
    {}
    
    TriMesh () : data (sizeof(TriMeshVertex), NULL)
    {}
    
    void addVertex (void *data);
    void addFaceGroup (MaterialID matID);
    void addFace (VertexID v1, VertexID v2, VertexID v3);
    void fromPoly (PolyMesh *m, TexMesh *uv);
  };


}//namespace GE
#pragma warning(pop)
#endif // __GESTATICMESH_H
