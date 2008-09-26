#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (DMesh);
  DEFINE_CLASS (DMesh::Vertex);
  DEFINE_CLASS (DMesh::HalfEdge);
  DEFINE_CLASS (DMesh::Edge);
  DEFINE_CLASS (DMesh::Face);

  typedef DMesh::Vertex Vertex;
  typedef DMesh::HalfEdge HalfEdge;
  typedef DMesh::Edge Edge;
  typedef DMesh::Face Face;

  DMesh::DMesh()
  {
    
    setClasses(
      Class(Vertex),
      Class(HalfEdge),
      Class(Edge),
      Class(Face));
    
    useSmoothGroups = true;
    shadingModel = SHADING_SMOOTH;
    
    for (MaterialId m=0; m<GE_MAX_MATERIAL_ID; ++m)
      facesPerMaterial[m] = 0;
  }
  
  /*------------------------------------------------
  Make sure a freshly inserted half edge has got
  a valid smooth normal even before updateNormals()
  is called.
  --------------------------------------------------*/

  void DMesh::insertHalfEdge(HMesh::HalfEdge *he)
  {
    HMesh::insertHalfEdge(he);
    ((DMesh::HalfEdge*)he)->snormal = &dummySmoothNormal;
  }
  
  /*--------------------------------------------------
  Material ID handling
  ----------------------------------------------------*/
  
  void DMesh::addMaterialId(MaterialId m)
  {
    facesPerMaterial[m]++;
    if (facesPerMaterial[m] == 1)
      if (!materialsUsed.contains(m))
        materialsUsed.pushFront(m);
  }
  
  void DMesh::subMaterialId(MaterialId m)
  {
    facesPerMaterial[m]--;
    if (facesPerMaterial[m] == 0)
      materialsUsed.remove(m);
  }
  
  void DMesh::insertFace(HMesh::Face *f)
  {
    HMesh::insertFace(f);
    addMaterialId(((DMesh::Face*)f)->materialId());
  }
  
  ListHandle DMesh::deleteFace(HMesh::Face *f)
  {
    subMaterialId(((DMesh::Face*)f)->materialId());
    return HMesh::deleteFace(f);
  }
  
  void DMesh::setMaterialId(Face *f, MaterialId id)
  {
    if (id == f->materialId()) return;
    subMaterialId(f->materialId());
    addMaterialId(id);
    f->matId = id;
  }


  /*------------------------------------------------
  Calculates face normal from first three vertices
  --------------------------------------------------*/

  void DMesh::updateFaceNormal(Face *f)
  {
    Vector3 &p1 = f->firstHedge()->dstVertex()->point;
    Vector3 &p2 = f->firstHedge()->nextHedge()->dstVertex()->point;
    Vector3 &p3 = f->firstHedge()->nextHedge()->nextHedge()->dstVertex()->point;
    Vector3 s1 = p2 - p1;
    Vector3 s2 = p3 - p1;
    f->normal = Vector::Cross(s1, s2).normalize();
    f->center = (p1 + p2 + p3) / 3;
  }

  /*-----------------------------------------------
  Calculates per-face normals for given vertex.
  Smoothing groups are not taken into account
  which results in a single normal being used for
  each adjacent face.
  -------------------------------------------------*/

  void DMesh::updateVertNormal(Vertex *vert)
  {
    int count = 0;
    Vector3 sum;
    VertFaceIter f;

    //Pass1: accumulate adjacent face normals
    for (f.begin(vert); !f.end(); ++f) {
      sum += f->normal;
      count++; }

    //Average the final normal vector
    if (count > 0) sum /= (Float)count;
    smoothNormals.pushBack( SmoothNormal( sum ));

    //Pass2: store normal into each half-edge
    for (f.begin(vert); !f.end(); ++f) {
      f.hedgeToVertex()->snormal = &smoothNormals.last();
    }
  }


  /*-----------------------------------------------
  This class provides basic operations for adding
  faces to and merging smoothing groups to make
  the normal calculation code more readable.
  -------------------------------------------------*/

  class SmoothGroup
  {
  public:
    Uint32 mask;
    Vector3 normal;
    int faceCount;
    Face *zeroFace;

    SmoothGroup() {
      mask = 0;
      faceCount = 0;
      zeroFace = NULL;
    }

    SmoothGroup(Face *f) {
      mask = f->smoothGroups;
      normal = f->normal;
      faceCount = 1;
      zeroFace = (mask==0) ? f : NULL;
    }

    SmoothGroup& operator+= (SmoothGroup &g) {
      mask |= g.mask;
      normal += g.normal;
      faceCount += g.faceCount;
      return *this;
    }

    SmoothGroup& operator+= (Face *f) {
      mask |= f->smoothGroups;
      normal += f->normal;
      faceCount++;
      return *this;
    }

    bool operator== (Face *f) {
      //No two faces with 0 smoothing flags are considered to be in
      //the same group. However, zeroFace is stored so that a face with
      //0 flag matches it's own SmoothGroup if tested again upon creation
      return ((mask & f->smoothGroups) != 0) || (zeroFace == f);
    }
  };


  /*-----------------------------------------------
  Calculates per-face normals for given vertex
  according to smoothing groups and stores them
  into half-edges pointing to that vertex in each
  of the adjacent faces.
  -------------------------------------------------*/

  void DMesh::updateVertNormalGroups(Vertex *vert)
  {
    DMesh::VertFaceIter f;
    ArraySet<SmoothGroup> groups(8);
    int g = 0;

    //Pass1: Walk the adjacent faces and merge smooth groups
    for (f.begin(vert); !f.end(); ++f) {
      
      //Walk the existing groups
      int matchCount = 0;
      SmoothGroup *firstMatch = NULL;
      for (g=0; g<groups.size(); ) {

        //Add normal if any common smoothing group
        if (groups[g] == *f) {
          groups[g] += (*f);

          //Store first match
          if (++matchCount == 1) {
            firstMatch = &groups[g];
          }else {

            //Merge multiple matches
            (*firstMatch) += groups[g];
            groups.removeAt(g);
            continue;
          }
        }

        g++; } //Walk groups

      //Create a new group
      if (matchCount == 0)
        groups.add(SmoothGroup(*f));

    }//Walk faces


    //MidPass: Average out the group normals
    //and add to smooth normal list
    int firstNormalId = smoothNormals.size();
    for (g=0; g<groups.size(); ++g) {
      groups[g].normal /= (Float)groups[g].faceCount;
      smoothNormals.pushBack( SmoothNormal( groups[g].normal ));
    }


    //Pass2: Apply unique normals to faces
    for (f.begin(vert); !f.end(); ++f) {

      //Find the matching merged group
      for (g=0; g<groups.size(); ++g) {
        if (groups[g] == *f) {
          f.hedgeToVertex()->snormal =
            &smoothNormals[ firstNormalId + g ];
          break;
        }}
    }
  }


  /*--------------------------------------------------
  Updates face and vertex normals for the whole mesh
  ----------------------------------------------------*/

  void DMesh::updateNormals()
  {
    smoothNormals.clear();

    for (DMesh::FaceIter f(this); !f.end(); ++f)
      updateFaceNormal(*f);

    if (useSmoothGroups) {

      for (DMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalGroups(*v);

    }else{

      for (DMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormal(*v);
    }
  }



}//namespace GE
