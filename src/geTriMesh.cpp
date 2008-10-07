#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;


namespace GE
{
  DEFINE_CLASS (TriMesh);

  /*
  ---------------------------------------------------
  When packing a dynamic editable mesh into a static
  representation, the final memory layout of data
  has to be appropriate for OpenGL rendering calls.
  Since glDrawElements doesn't take separate indices
  for points, texture coordinates and normals, we
  have to produce multiple vertex variants to
  represent the same dynamic vertex in many of its
  incident faces.
  
  The algorithm walks all the incident faces of each
  vertex and checks whether the normal or texture
  vertex pointers differ for any of the faces.

  In the first pass the pointers to the respective
  vertices in the dynamic UV mesh are copied into
  the adjacent face's incoming halfedge helper tag.
  These are later inspected to determine the
  differences in UV coordinates based on the
  texture vertex pointers.
  
  When an adjacent face is visited, that tag is
  replaced with the index into the array of final
  output vertices (existing or new one).
  
  If a pointer to a Vertex array is given, it is
  filled with pointers to vertices in the original
  polygonal mesh for each produced triangle mesh
  vertex. This efficiently creates a mapping
  between the TriMesh and PolyMesh vertices for
  use in subsequent algorithms.
  --------------------------------------------------*/
  
  struct UniqueVertex
  {
    StaticId    staticId;
    MaterialId  materialId;
    void*       uvertexPtr;
    void*       vnormalPtr;
  };

  void TriMesh::fromPoly (PolyMesh *m, TexMesh *um)
  {
    PolyMesh::FaceIter f;
    PolyMesh::FaceHedgeIter h;
    TexMesh::FaceIter uf;
    TexMesh::FaceHedgeIter uh;
    ArrayList<UniqueVertex> uniqVerts;
    StaticId nextStaticId = 0;
    
    /*
    if (Kernel::Instance->hasRangeElements) {
      printf( "Max verts: %d\n", Kernel::Instance->maxElementsVertices );
      printf( "Max indices: %d\n", Kernel::Instance->maxElementsIndices );
    }
    */

    data.clear();
    indices.clear();

    //Store UV pointers into vert-per-face hedges
    //(These are later replaced with static vertex IDs)
    for (f.begin(m), uf.begin(um); !f.end(); ++f, ++uf) {
      for (h.begin(*f), uh.begin(*uf); !h.end(); ++h, ++uh) {
        if (!uh.end()) h->tag.ptr = uh->dstVertex();
        else h->tag.ptr = NULL;
      }}

    //Walk all the vertices of the mesh
    for (PolyMesh::VertIter v(m); !v.end(); ++v)
    {
      //An array for unique vertices
      uniqVerts.clear();
      
      //Walk adjacent faces and output unique vertex variants
      for (PolyMesh::VertFaceIter vf(*v); !vf.end(); ++vf) {
        PolyMesh::HalfEdge *vfh = vf.hedgeToVertex();
        
        //Walk existing vertex variants
        bool existingFound = false;
        for (int u=uniqVerts.size()-1; u>=0; --u)
        {
          //Check for match
          if (uniqVerts[u].materialId == vf->materialId() &&
              uniqVerts[u].uvertexPtr == vfh->tag.ptr &&
              uniqVerts[u].vnormalPtr == vfh->vertexNormal())
          {
            //Assign existing static ID
            vfh->tag.id = uniqVerts[u].staticId;
            existingFound = true;
            break;
          }
        }
        
        //Output a new vertex variant
        if (!existingFound)
        {
          //Uniqueness info
          UniqueVertex uniq;
          uniq.materialId = vf->materialId();
          uniq.uvertexPtr = vfh->tag.ptr;
          uniq.vnormalPtr = vfh->vertexNormal();
          uniq.staticId = nextStaticId;
          uniqVerts.pushBack( uniq );
          
          //Invoke the vertex exporter
          vertexFromPoly (*v, vfh->vertexNormal(),
                          (TexMesh::Vertex*) vfh->tag.ptr);
          
          //Assign new static ID
          vfh->tag.id = nextStaticId++;
        }
        
      }//Walk adjacent faces
    }//Walk all vertices

    
    //Walk the materials used by the mesh
    for (LinkedList<MaterialId>::Iterator mid=m->materialsUsed.begin();
         mid != m->materialsUsed.end(); ++mid)
    {
      //Initialize material index group
      IndexGroup grp;
      grp.materialId = *mid;
      grp.start = indices.size();
      
      //Walk faces of current material and invoke the exporter
      for (PolyMesh::MaterialFaceIter mf( m, *mid ); !mf.end(); ++mf)
        faceFromPoly( *mf );
      
      //Count the number of exported indices
      grp.count = indices.size() - grp.start;
      groups.pushBack( grp );
    }
  }
  
  /*
  -------------------------------------------------------
  This is the default triangle vertex exporter function.
  It generates the triangle mesh data from the polygonal
  mesh input structures. The data exported are vertex
  coordinate, vertex normal and texture coordinate.
  -------------------------------------------------------*/
  
  void TriMesh::vertexFromPoly (PolyMesh::Vertex *polyVert,
                                PolyMesh::VertexNormal *polyNormal,
                                TexMesh::Vertex *texVert)
  {
    //Add UV coord to data
    if (texVert == NULL) {
      data.pushBack( 0.0f );
      data.pushBack( 0.0f );
    }else{
      data.pushBack( texVert->point.x );
      data.pushBack( texVert->point.y ); }
    
    //Add normal coord to data
    data.pushBack( polyNormal->coord.x );
    data.pushBack( polyNormal->coord.y );
    data.pushBack( polyNormal->coord.z );
    
    //Add vertex coord to data
    data.pushBack( polyVert->point.x );
    data.pushBack( polyVert->point.y );
    data.pushBack( polyVert->point.z );
  }
  
  /*
  ----------------------------------------------------
  This is the default triangle face exporter function.
  It triangulates the input polygon using a simple
  ear-cut algorithm and stores the resulting indices.
  ----------------------------------------------------*/
  
  void TriMesh::faceFromPoly (PolyMesh::Face *polyFace)
  {
    PolyMesh::HalfEdge *cur = polyFace->firstHedge();
    PolyMesh::HalfEdge *prev = cur->prevHedge();
    PolyMesh::HalfEdge *next = cur->nextHedge();
    
    do {
      
      indices.pushBack( prev->tag.id );
      indices.pushBack( cur->tag.id );
      indices.pushBack( next->tag.id );
      
      cur = cur->nextHedge();
      next = next->nextHedge();
      
    } while (next != prev);
  }

}//namespace GE
