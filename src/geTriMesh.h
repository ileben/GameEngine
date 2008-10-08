#ifndef __GESTATICMESH_H
#define __GESTATICMESH_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*------------------------------------------------------------
   * Static mesh is highly optimized for rendering. Note that
   * since OpenGL doesn't allow separate indexes for vertex
   * and texture/color/normal arrays, certain information has
   * to be duplicated many times in the respective array in
   * case another varies across different faces, adjacent to
   * a certain vertex (like normals when flat-shading is used).
   * This also makes the StaticMesh less appropriate for direct
   * target of skinning result - since almost every uvw mapping
   * contains many separated mesh patches, the vertices along
   * the seam have different uvw coordinate for each adjacent
   * face and thus the vertex coordinate has to be copied for
   * each different uvw coordinate of that vertex. In the end
   * this raises the complexity of the algorithm, but is not
   * much faster than computing the skinning result in the
   * dynamic mesh itself and then recopying it to a static
   * mesh structure in case the rendering optimization would
   * surpass the time needed for copying of data.
   *------------------------------------------------------------*/
  
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
    DECLARE_CALLBACK( CLSEVT_SERIALIZE, serialize );
    DECLARE_END;
    
  public:
    
    struct IndexGroup
    {
      MaterialID materialID;
      VertexID   start;
      VertexID   count;
    };
    
    GenericArrayList *data;
    ArrayListT <Uint32> *indices;
    ArrayListT <IndexGroup> *groups;
    
  protected:
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::VertexNormal *polyNormal,
                                 TexMesh::Vertex *texVert);
    
    virtual void faceFromPoly (PolyMesh::Face *polyFace);
    
  public:
        
    void serialize (void *sm)
    {
      ( (SM*)sm )->resourcePtr( &data );
      ( (SM*)sm )->resourcePtr( &indices );
      ( (SM*)sm )->resourcePtr( &groups );
    }
    
    TriMesh( SerializeManager *sm )
    {}
    
    TriMesh()
    {
      data = new GenericArrayList( sizeof( TriMeshVertex ));
      indices = new ArrayListT <Uint32>;
      groups = new ArrayListT <IndexGroup>;
    }
    
    TriMesh( Uint32 vertexSize )
    {
      data = new GenericArrayList( vertexSize );
      indices = new ArrayListT <Uint32>;
      groups = new ArrayListT <IndexGroup>;
    }
    
    virtual ~TriMesh ()
    {
      delete data;
      delete indices;
      delete groups;
    }
    
    void addVertex( void *data );
    void addFaceGroup( MaterialID matID );
    void addFace( VertexID v1, VertexID v2, VertexID v3 );
    void fromPoly (PolyMesh *m, TexMesh *uv );
  };


}//namespace GE
#pragma warning(pop)
#endif // __GESTATICMESH_H
