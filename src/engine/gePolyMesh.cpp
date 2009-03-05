#include "gePolyMesh.h"

namespace GE
{
  DEFINE_CLASS (PolyMesh);
  DEFINE_CLASS (PolyMesh::Vertex);
  DEFINE_CLASS (PolyMesh::HalfEdge);
  DEFINE_CLASS (PolyMesh::Edge);
  DEFINE_CLASS (PolyMesh::Face);

  typedef PolyMesh::Vertex Vertex;
  typedef PolyMesh::HalfEdge HalfEdge;
  typedef PolyMesh::Edge Edge;
  typedef PolyMesh::Face Face;

  PolyMesh::PolyMesh ()
  {
    useSmoothGroups = true;
    
    for (MaterialID m=0; m<GE_MAX_MATERIAL_ID; ++m)
      facesPerMaterial [m] = 0;
  }
  
  /*
  --------------------------------------------------
  Make sure a freshly inserted half edge has got
  a valid smooth normal even before updateNormals()
  is called.
  --------------------------------------------------*/

  void PolyMesh::insertHalfEdge (HMesh::HalfEdge *he)
  {
    HMesh::insertHalfEdge(he);
    ((PolyMesh::HalfEdge*)he)->vnormal = &dummyVertexNormal;
  }
  
  /*
  ----------------------------------------------------
  Material ID handling
  ----------------------------------------------------*/
  
  void PolyMesh::addMaterialID (MaterialID m)
  {
    facesPerMaterial[ m ]++;
    if( facesPerMaterial[ m ] == 1 )
      if(! materialsUsed.contains( m ))
        materialsUsed.pushFront( m );
  }
  
  void PolyMesh::subMaterialID (MaterialID m)
  {
    facesPerMaterial[ m ]--;
    if( facesPerMaterial[ m ] == 0 )
      materialsUsed.remove( m );
  }
  
  void PolyMesh::insertFace (HMesh::Face *f)
  {
    HMesh::insertFace(f);
    addMaterialID( ((PolyMesh::Face*) f)->materialID() );
  }
  
  ListHandle PolyMesh::deleteFace (HMesh::Face *f)
  {
    subMaterialID( ((PolyMesh::Face*) f)->materialID() );
    return HMesh::deleteFace(f);
  }
  
  void PolyMesh::setMaterialID (Face *f, MaterialID id)
  {
    if( id == f->materialID() ) return;
    subMaterialID( f->materialID() );
    addMaterialID( id );
    f->matId = id;
  }

  /*
  --------------------------------------------------
  Calculates face normal from first three vertices
  --------------------------------------------------*/

  void PolyMesh::updateFaceNormal (Face *f)
  {
    Vector3 &p1 = f->firstHedge()->dstVertex()->point;
    Vector3 &p2 = f->firstHedge()->nextHedge()->dstVertex()->point;
    Vector3 &p3 = f->firstHedge()->nextHedge()->nextHedge()->dstVertex()->point;
    Vector3 s1 = p2 - p1;
    Vector3 s2 = p3 - p1;
    f->normal = Vector::Cross(s1, s2).normalize();
    f->center = (p1 + p2 + p3) / 3;
  }
  
  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex.
  Normals are not averaged over the adjacent
  faces which results in a different normal
  being used for each adjacent face.
  -------------------------------------------------*/
  
  void PolyMesh::updateVertNormalFlat (Vertex *vert)
  {
    VertFaceIter f;
    
    //Take the normal off each incident face and store it
    for (f.begin(vert); !f.end(); ++f) {
      vertexNormals.pushBack (VertexNormal (f->normal));
      vertexNormals.last().vert = vert;
      f.hedgeToVertex()->vnormal = &vertexNormals.last();
    }
  }
  
  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex.
  Smoothing groups are not taken into account
  which results in a single normal being used for
  each adjacent face.
  -------------------------------------------------*/
  
  void PolyMesh::updateVertNormalSmooth (Vertex *vert)
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
    vertexNormals.pushBack (VertexNormal (sum));
    vertexNormals.last().vert = vert;

    //Pass2: store normal into each half-edge
    for (f.begin(vert); !f.end(); ++f) {
      f.hedgeToVertex()->vnormal = &vertexNormals.last();
    }
  }


  /*
  -------------------------------------------------
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

    SmoothGroup () {
      mask = 0;
      faceCount = 0;
      zeroFace = NULL;
    }

    SmoothGroup (Face *f) {
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
      //0 flag matches it's own SmoothGroup if tested again after creation
      return ((mask & f->smoothGroups) != 0) || (zeroFace == f);
    }
  };


  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex
  according to smoothing groups and stores them
  into half-edges pointing to that vertex in each
  of the adjacent faces.
  -------------------------------------------------*/

  void PolyMesh::updateVertNormalGroups(Vertex *vert)
  {
    PolyMesh::VertFaceIter f;
    ArraySet<SmoothGroup> groups(8);
    int g = 0;

    //Pass1: Walk the adjacent faces and merge smooth groups
    for (f.begin(vert); !f.end(); ++f)
    {
      //Walk the existing groups
      int matchCount = 0;
      SmoothGroup *firstMatch = NULL;
      for (g=0; g<groups.size(); )
      {
        //Add normal if any common smoothing group
        if (groups[g] == *f) {
          groups[g] += (*f);

          //Store first match
          if (++matchCount == 1) {
            firstMatch = &groups[g];
          }else {

            //Merge multiple matches
            (*firstMatch) += groups[g];
            groups.removeAt (g);
            continue;
          }
        }

        g++; } //Walk groups

      //Create a new group
      if (matchCount == 0)
        groups.add (SmoothGroup (*f));

    }//Walk faces


    //MidPass: Average out the group normals
    //and add to vertex normal list
    int firstNormalId = vertexNormals.size();
    for (g=0; g<groups.size(); ++g)
    {
      groups[g].normal /= (Float)groups[g].faceCount;
      vertexNormals.pushBack (VertexNormal (groups[g].normal));
      vertexNormals.last().vert = vert;
    }
    
    //Pass2: Apply unique normals to faces
    for (f.begin(vert); !f.end(); ++f)
    {
      //Find the matching merged group
      for (g=0; g<groups.size(); ++g) {
        if (groups[g] == *f) {
          f.hedgeToVertex()->vnormal =
            &vertexNormals [firstNormalId + g];
          break;
        }}
    }
  }


  /*
  ----------------------------------------------------
  Updates face and vertex normals for the whole mesh
  ----------------------------------------------------*/

  void PolyMesh::updateNormals (ShadingModel shadingModel)
  {
    vertexNormals.clear();
    
    for (PolyMesh::FaceIter f(this); !f.end(); ++f)
      updateFaceNormal(*f);
    
    switch (shadingModel)
    {
    case SHADING_FLAT:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalFlat (*v);
      
      break;
    case SHADING_SMOOTH:
      if (useSmoothGroups) {
        
        for (PolyMesh::VertIter v(this); !v.end(); ++v)
          updateVertNormalGroups(*v);
        
      }else{
        
        for (PolyMesh::VertIter v(this); !v.end(); ++v)
          updateVertNormalSmooth(*v);
      }
      
      break;
    }
  }



}//namespace GE
