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

  struct UniqueData
  {
    TexMesh::Vertex *texVertex;
    VertexID vertexID;
  };
  
  void TriMesh::fromPoly (PolyMesh *m, TexMesh *um)
  {
    PolyMesh::FaceIter f;
    PolyMesh::FaceHedgeIter fh;
    TexMesh::FaceIter uf;
    TexMesh::FaceHedgeIter ufh;
    ArrayList<UniqueVertex> uniqVerts;
    VertexID nextVertexID = 0;

    data.clear();
    indices.clear();

    //Allocate structures for vert-per-face unique data
    //and store texture vertex pointers
    for (f.begin(m), uf.begin(um); !f.end(); ++f, ++uf) {
      for (fh.begin(*f), ufh.begin(*uf); !fh.end(); ++fh, ++ufh) {
        UniqueData *data = new UniqueData;
        if (!ufh.end()) data->texVertex = ufh->dstVertex();
        else data->texVertex = NULL;
        fh->tag.ptr = data;
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
        for (PolyMesh::VertFaceIter vf2(*v); vf2 != vf; ++vf2) {
          PolyMesh::HalfEdge *vfh2 = vf2.hedgeToVertex();

          //Check for match
          if (isPolyVertexEqual( *v, vfh, vfh2))
          {
            //Assign existing vertex ID
            ((UniqueData*)vfh->tag.ptr)->vertexID =
              ((UniqueData*)vfh2->tag.ptr)->vertexID;
            existingFound = true;
            break;
          }
        }
        
        //Output a new vertex variant
        if (!existingFound)
        {
          //Invoke the vertex exporter
          vertexFromPoly (*v, vfh, ((UniqueData*)vfh->tag.ptr)->texVertex);
          
          //Assign new vertex ID
          ((UniqueData*)vfh->tag.ptr)->vertexID = nextVertexID++;
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
        faceFromPoly( m, *mf );
    }

    //Deallocate vert-per-face unique data
    for (f.begin(m); !f.end(); ++f)
      for (fh.begin(*f); !fh.end(); ++fh)
        delete fh->tag.ptr;
  }

  bool TriMesh::isPolyVertexEqual (PolyMesh::Vertex *polyVert,
                                   PolyMesh::HalfEdge *polyHedge1,
                                   PolyMesh::HalfEdge *polyHedge2)
  {
    if (((UniqueData*)polyHedge1->tag.ptr)->texVertex !=
        ((UniqueData*)polyHedge2->tag.ptr)->texVertex)
      return false;

    if (polyHedge1->parentFace()->materialID() !=
        polyHedge2->parentFace()->materialID())
      return false;

    if (polyHedge1->vertexNormal() !=
        polyHedge2->vertexNormal())
      return false;

    return true;
  }
  
  /*
  -------------------------------------------------------
  This is the default triangle vertex exporter function.
  It generates the triangle mesh data from the polygonal
  mesh input structures. The data exported are vertex
  coordinate, vertex normal and texture coordinate.
  -------------------------------------------------------*/
  
  void TriMesh::vertexFromPoly (PolyMesh::Vertex *polyVert,
                                PolyMesh::HalfEdge *polyHedge,
                                TexMesh::Vertex *texVert)
  {
    Vertex vert;
    
    if (texVert != NULL)
      vert.texcoord = texVert->point;
    
    vert.normal = polyHedge->vertexNormal()->coord;
    vert.point = polyVert->point;
    
    addVertex( &vert );
  }
  
  /*
  ----------------------------------------------------
  This is the default triangle face exporter function.
  It triangulates the input polygon using a simple
  ear-cut algorithm and stores the resulting indices.
  ----------------------------------------------------*/
  
  void TriMesh::faceFromPoly (PolyMesh *polyMesh,
                              PolyMesh::Face *polyFace)
  {
    //Walk the triangles of the face
    for (PolyMesh::FaceTriIter ft( polyMesh, polyFace ); !ft.end(); ++ft)
    {
      addFace(
        ((UniqueData*)ft->hedgeToVertex(0)->tag.ptr)->vertexID,
        ((UniqueData*)ft->hedgeToVertex(1)->tag.ptr)->vertexID,
        ((UniqueData*)ft->hedgeToVertex(2)->tag.ptr)->vertexID
      );
    }
  }

  /*
  ----------------------------------------------------
  Helper functions for data retrieval
  ----------------------------------------------------*/

  VertexID TriMesh::getCornerIndex (UintSize group, UintSize face, UintSize corner)
  {
    return indices[ groups[ group ].start + face*3 + corner ];
  }

  TriMesh::Vertex* TriMesh::getVertex (UintSize index)
  {
    return (TriMesh::Vertex*) data[ index ];
  }

  UintSize TriMesh::getVertexCount ()
  {
    return data.size();
  }

  UintSize TriMesh::getFaceCount ()
  {
    return indices.size() / 3;
  }
  
  UintSize TriMesh::getGroupFaceCount (UintSize group)
  {
    return groups[ group ].count / 3;
  }

  void TriMesh::sendToGpu ()
  {
    if (!isOnGpu)
    {
      glGenBuffers( 1, &dataVBO );
      glGenBuffers( 1, &indexVBO );
    }

    glBindBuffer( GL_ARRAY_BUFFER, dataVBO );
    glBufferData( GL_ARRAY_BUFFER, data.size() * data.elementSize(), data.buffer(), GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * indices.elementSize(), indices.buffer(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    isOnGpu = true;
  }

  /*
  ----------------------------------------------------
  Copies a part of the mesh to another mesh
  ----------------------------------------------------*/

  SuperToSubMesh::SuperToSubMesh (TriMesh *super)
  {
    this->super = super;
  }

  UintSize SuperToSubMesh::subMeshForFace (UintSize superGroup, UintSize superFace)
  {
    return 0;
  }

  TriMesh* SuperToSubMesh::newSubMesh (UintSize subMeshID)
  {
    return new TriMesh;
  }

  void SuperToSubMesh::newSubFace (UintSize subMeshID, VertexID subID1, VertexID subID2, VertexID subID3)
  {
    subs[ subMeshID ].mesh->addFace( subID1, subID2, subID3 );
  }

  void SuperToSubMesh::newSubVertex (UintSize subMeshID, VertexID superID)
  {
    TriMesh::Vertex *superVert = super->getVertex( superID );
    
    TriMesh::Vertex subVert;
    subVert.normal = superVert->normal;
    subVert.texcoord = superVert->texcoord;
    subVert.point = superVert->point;
    
    subs[ subMeshID ].mesh->addVertex( &subVert );
  }

  std::pair<bool,VertexID> SuperToSubMesh::getSubVertexID (UintSize subMeshID, VertexID superID)
  {
    std::pair<bool,VertexID> retval;
    Super2SubIter it = subs[ subMeshID ].super2subMap.find( superID );
    retval.first = (it != subs[ subMeshID ].super2subMap.end());
    retval.second = (retval.first ? it->second : 0);
    return retval;
  }

  void SuperToSubMesh::split()
  {
    VertexID subCorners[3];

    //Walk groups
    for (UintSize g=0; g<super->groups.size(); ++g)
    {
      //Walk faces in this group
      for (VertexID face=0; face<super->getGroupFaceCount(g); ++face)
      {
        UintSize subMeshID = subMeshForFace( g, face );
        
        //Make sure sub mesh ID is valid
        while (subMeshID >= subs.size())
          subs.pushBack( SubMeshInfo() );

        //Create new mesh if missing
        if (subs[ subMeshID ].mesh == NULL)
          subs[ subMeshID ].mesh = newSubMesh( subMeshID );

        //Create new material group if missing
        SubMeshInfo *sub = &subs[ subMeshID ];
        if (sub->mesh->groups.empty())
          sub->mesh->addFaceGroup( super->groups[g].materialID );
        else if (sub->mesh->groups.last().materialID != super->groups[g].materialID)
          sub->mesh->addFaceGroup( super->groups[g].materialID );

        //Walk corners of the face
        for (VertexID corner=0; corner<3; ++corner)
        {
          //Check if this vertex is missing in the sub mesh
          VertexID superID = super->getCornerIndex( g, face, corner );
          std::pair<bool,VertexID> subVertexID = getSubVertexID( subMeshID, superID );
          if (subVertexID.first == false)
          {
            //Copy vertex to sub mesh and map super to sub ID
            newSubVertex( subMeshID, superID );
            subCorners[ corner ] = sub->nextVertexID;
            sub->super2subMap[ superID ] = sub->nextVertexID;
            sub->nextVertexID++;
          }
          else
          {
            //Use existing sub ID
            subCorners[ corner ] = subVertexID.second;
          }
        }

        //Add face corners to sub mesh
        newSubFace( subMeshID, subCorners[0], subCorners[1], subCorners[2] );

      }//Walk faces
    }//Walk groups
  }

}//namespace GE
