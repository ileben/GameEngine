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
  
  typedef Uint32 StaticId;

  class GE_API_ENTRY SMesh : public Resource
  {
    DECLARE_SUBCLASS (SMesh, Resource); DECLARE_END;
    friend class Renderer;
    
  public:
    
    struct IndexGroup
    {
      MaterialId materialId;
      StaticId start;
      StaticId count;
    };
    
    OCC::ArrayList<Float> data;
    OCC::ArrayList<Uint32> indices;
    OCC::LinkedList<IndexGroup> groups;

  public:
    void fromDynamic(DMesh *m, UMesh *uv);
  };


}//namespace GE
#pragma warning(pop)
#endif // __GESTATICMESH_H
