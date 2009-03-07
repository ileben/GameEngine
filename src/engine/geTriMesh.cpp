#include "geTriMesh.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_SERIAL_CLASS( TriMesh, CLSID_TRIMESH );

  /*
  ---------------------------------------------------
  Adds vertex data to the buffer
  ---------------------------------------------------*/
  
  void TriMesh::addVertex( void *vertData )
  {
    data.pushBack( vertData );
  }
  
  /*
  -----------------------------------------------------
  Creates a new face group for the given material ID
  -----------------------------------------------------*/
  
  void TriMesh::addFaceGroup( MaterialID matID )
  {
    IndexGroup newGrp;
    newGrp.materialID = matID;
    newGrp.start = (VertexID) indices.size();
    newGrp.count = 0;
    groups.pushBack( newGrp );
  }
  
  /*
  -----------------------------------------------------
  Adds a face to the last created face group
  -----------------------------------------------------*/
  
  void TriMesh::addFace( VertexID v1, VertexID v2, VertexID v3 )
  {
    indices.pushBack( v1 );
    indices.pushBack( v2 );
    indices.pushBack( v3 );
    groups.last().count += 3;
  }
  
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
    VertexID    vertexID;
    MaterialID  materialID;
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
    VertexID nextVertexID = 0;
    
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
        for( int u=(int)uniqVerts.size()-1; u>=0; --u )
        {
          //Check for match
          if (uniqVerts[u].materialID == vf->materialID() &&
              uniqVerts[u].uvertexPtr == vfh->tag.ptr &&
              uniqVerts[u].vnormalPtr == vfh->vertexNormal())
          {
            //Assign existing vertex ID
            vfh->tag.id = uniqVerts[u].vertexID;
            existingFound = true;
            break;
          }
        }
        
        //Output a new vertex variant
        if (!existingFound)
        {
          //Uniqueness info
          UniqueVertex uniq;
          uniq.materialID = vf->materialID();
          uniq.uvertexPtr = vfh->tag.ptr;
          uniq.vnormalPtr = vfh->vertexNormal();
          uniq.vertexID = nextVertexID;
          uniqVerts.pushBack( uniq );
          
          //Invoke the vertex exporter
          vertexFromPoly (*v, vfh->vertexNormal(),
                          (TexMesh::Vertex*) vfh->tag.ptr);
          
          //Assign new vertex ID
          vfh->tag.id = nextVertexID++;
        }
        
      }//Walk adjacent faces
    }//Walk all vertices

    
    //Walk the materials used by the mesh
    for (LinkedList<MaterialID>::Iterator mid=m->materialsUsed.begin();
         mid != m->materialsUsed.end(); ++mid)
    {
      //Create a new face group for this material
      addFaceGroup( *mid );
      
      //Walk faces of current material and invoke the exporter
      for (PolyMesh::MaterialFaceIter mf( m, *mid ); !mf.end(); ++mf)
        faceFromPoly( *mf );
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
    Vertex vert;
    
    if (texVert != NULL)
      vert.texcoord = texVert->point;
    
    vert.normal = polyNormal->coord;
    vert.point = polyVert->point;
    
    addVertex( &vert );
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
      
      addFace( prev->tag.id, cur->tag.id, next->tag.id );
      
      cur = cur->nextHedge();
      next = next->nextHedge();
      
    } while (next != prev);
  }

  /*
  ----------------------------------------------------
  Helper functions for data retrieval
  ----------------------------------------------------*/

  TriMesh::Vertex* TriMesh::getVertex (int index)
  {
    return (TriMesh::Vertex*) data[ index ];
  }

}//namespace GE
