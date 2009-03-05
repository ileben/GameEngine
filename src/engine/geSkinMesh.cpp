#include "geSkinMesh.h"

namespace GE
{
  DEFINE_CLASS( SPolyMesh );
  DEFINE_CLASS( SPolyMesh::Vertex );
  DEFINE_CLASS( SPolyMesh::HalfEdge );
  DEFINE_CLASS( SPolyMesh::Edge );
  DEFINE_CLASS( SPolyMesh::Face );

  DEFINE_SERIAL_CLASS (SkinMesh, CLSID_SKINMESH);


  void SkinTriMesh::vertexFromPoly (PolyMesh::Vertex *polyVert,
                                    PolyMesh::VertexNormal *polyNormal,
                                    TexMesh::Vertex *texVert)
  {
    SPolyMesh::Vertex *skinVert = (SPolyMesh::Vertex*) polyVert;
    SkinTriVertex triVert;
    
    if (texVert != NULL)
      triVert.texcoord = texVert->point;
    
    triVert.normal = polyNormal->coord;
    triVert.point = polyVert->point;
    
    for (int i=0; i<4; ++i)
      triVert.boneIndex[ i ] = skinVert->boneIndex[ i ];
    
    for (int i=0; i<4; ++i)
      triVert.boneWeight[ i ] = skinVert->boneWeight[ i ];
    
    addVertex( &triVert );
  }

}//namespace GE
