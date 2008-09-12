#ifndef __GEDYNAMICMESH_H
#define __GEDYNAMICMESH_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{

  /*======================================================
   *
   * Shading model determines whether face or per-vertex
   * normals will be used when drawing this mesh.
   *
   *======================================================*/

  enum ShadingModel
  {
    SHADING_FLAT         = 1,
    SHADING_SMOOTH       = 2
  };
  
  /*=========================================================
   *
   * MaterialId is an index into sub-material array.
   * Max value is clamped to one less then max representable
   * so it is possible to hit the end value when iterating
   * over the array using MaterialId type.
   *
   *=========================================================*/
  
  typedef Uint8 MaterialId;
  #define GE_MAX_MATERIAL_ID 255
  

  /*===========================================================
   *
   * DynamicMesh is derived from a HMesh instantiation with
   * custom traits to hold data needed for animation and
   * rendering of the avatars. The base for this mesh allows
   * fast adjacency queries, necessary editing, calculation of
   * normals and many other mesh operations.
   *
   *===========================================================*/
  
  class GE_API_ENTRY DMesh : public HMesh
  {
    friend class SMesh;
    friend class LoaderObj;
    friend class Renderer;
    DECLARE_SUBCLASS (DMesh, HMesh);
    DECLARE_END;
    
  private:

    /*---------------------------------------------
    Smooth normals are dynamically allocated like
    the rest of the mesh entities and referenced
    by halfedges via pointers. This minimizes the
    memory space when many faces belong to same
    smoothing groups. When inserted into mesh,
    each new halfedge is set to point to a dummy
    normal, so it is still valid before normals
    get updated.
    ----------------------------------------------*/

    class SmoothNormal
    {
    public:
      ItemTag tag;
      Vector3 coord;
      SmoothNormal() {};
      SmoothNormal(const Vector3 &c) {coord = c;}
    };

    bool useSmoothGroups;
    ShadingModel shadingModel;
    SmoothNormal dummySmoothNormal;
    OCC::DynamicArrayList<SmoothNormal> smoothNormals;
    virtual void insertHalfEdge(HMesh::HalfEdge *he);
    
    /*--------------------------------------------
    We keep track of all the material IDs used on
    this mesh so the renderer knows how many times
    to traverse it.
    ----------------------------------------------*/
    
  private:
    int facesPerMaterial[GE_MAX_MATERIAL_ID];
    OCC::LinkedList<MaterialId> materialsUsed;
    void addMaterialId(MaterialId m);
    void subMaterialId(MaterialId m);
    virtual void insertFace(HMesh::Face *f);
    virtual ListHandle deleteFace(HMesh::Face *f);
    

    /*----------------------------------------------
    Custom mesh entities
    ------------------------------------------------*/

  public:
    
    class Vertex;
    class Edge;
    class HalfEdge;
    class Face;

    
    class GE_API_ENTRY Vertex : public VertexBase <DMesh,HMesh> {
      DECLARE_SUBCLASS (Vertex, HMesh::Vertex); DECLARE_END;
    public:
      Vector3 point;
    };

    class GE_API_ENTRY Edge : public EdgeBase <DMesh,HMesh> {
      DECLARE_SUBCLASS (Edge, HMesh::Edge); DECLARE_END;
    };

    /*------------------------------------------------------
    Half edge holds uvw coordinate and final normal for the
    target vertex with regard to its face. These can differ
    from one to another face for each vertex in case the
    adjacent faces are detached in the uvw map and/or the
    two faces are not in the same smoothing group
    -------------------------------------------------------*/

    class GE_API_ENTRY HalfEdge : public HalfEdgeBase <DMesh,HMesh> {
      DECLARE_SUBCLASS (HalfEdge, HMesh::HalfEdge); DECLARE_END;
    public:
      SmoothNormal *snormal;
      HalfEdge() {snormal = NULL;}
      INLINE SmoothNormal* smoothNormal() { return snormal; }
    };
    
    /*--------------------------------------------------
    Bits in the 32-bit field define whether the Face
    belongs to any of the 32 smoothing groups.
    MaterialId is an index into material's sub-material
    array.
    ----------------------------------------------------*/

    class GE_API_ENTRY Face : public FaceBase <DMesh,HMesh> {
      DECLARE_SUBCLASS (Face, HMesh::Face); DECLARE_END;
      friend class DMesh;
    private:
      Uint8 matId;
    public:
      Vector3 center;
      Vector3 normal;
      Uint32 smoothGroups;
      Face() {smoothGroups = 0; matId = 0;}
      Uint8 materialId() {return matId;}
    };

    //Data and adjancency iterators
    #include "geHmeshDataiter.h"
    #include "geHmeshAdjiter.h"
    #include "geDMeshIters.h"


  public:

    DMesh();
    
    //When true causes smoothing groups to be taken
    //into account when calculating vertex normals
    void setUseSmoothGroups(bool enabled) {
      useSmoothGroups = enabled;
    }

    bool getUseSmoothGroups() {
      return useSmoothGroups;
    }

    //When true the mesh is rendered using per-vertex
    //normals which are interpolated over triangles
    void setShadingModel(ShadingModel model) {
      shadingModel = model;
    }

    ShadingModel getShadingModel() {
      return shadingModel;
    }

  private:

    void updateFaceNormal(Face *f);
    void updateVertNormal(Vertex *v);
    void updateVertNormalGroups(Vertex *v);

  public:

    void updateNormals();
    void setMaterialId(Face *f, MaterialId id);
  };
  

  //class Bone
  //- ID
  //- origin
  //- rotation
  //- children->*
  
  //class Skin
  //- bones(ID?)->*
  //- weight*
  //- bindToSkeleton(rootBone) ?
  
  //class SkeletonAnimation
  //- boneID - keys*/values*
  //- bindToSkeleton(rootBone) { boneID <= bone-> }
  //- play(), setOffset(), ...


}//namespace GE
#pragma warning(pop)
#endif//__GEDYNAMICMESH_H
