#include "geSkinMesh.h"

namespace GE
{
  DEFINE_CLASS( SPolyMesh );
  DEFINE_CLASS( SPolyMesh::Vertex );
  DEFINE_CLASS( SPolyMesh::HalfEdge );
  DEFINE_CLASS( SPolyMesh::Edge );
  DEFINE_CLASS( SPolyMesh::Face );
  
  DEFINE_SERIAL_CLASS( SkinTriMesh, CLSID_SKINTRIMESH );
  //DEFINE_SERIAL_CLASS( SkinMesh, CLSID_SKINMESH );


  void SkinTriMesh::vertexFromPoly (PolyMesh::Vertex *polyVert,
                                    PolyMesh::HalfEdge *polyHedge,
                                    TexMesh::Vertex *texVert)
  {
    SPolyMesh::Vertex *skinVert = (SPolyMesh::Vertex*) polyVert;
    Vertex triVert;
    
    if (texVert != NULL)
      triVert.texcoord = texVert->point;
    
    triVert.normal = polyHedge->vertexNormal()->coord;
    triVert.point = polyVert->point;
    
    for (int i=0; i<4; ++i)
      triVert.boneIndex[ i ] = skinVert->boneIndex[ i ];
    
    for (int i=0; i<4; ++i)
      triVert.boneWeight[ i ] = skinVert->boneWeight[ i ];
    
    addVertex( &triVert );
  }

  /*
  -------------------------------------------------------------
  Algorithm that copies faces of a mesh into a sub mesh
  and remaps the bone indices until the bone limit is reached
  -------------------------------------------------------------*/

  void SkinSuperToSubMesh::splitByBoneLimit (Uint32 maxBonesPerMesh)
  {
    maxBones = maxBonesPerMesh;
    subMeshCount = 0;
    SuperToSubMesh::split();
  }

  TriMesh* SkinSuperToSubMesh::newSubMesh (UintSize subMeshID)
  { 
    SkinSubMeshInfo sub;
    sub.mesh = new SkinTriMesh;
    subs.pushBack( sub );
    subMeshCount++;

    return sub.mesh;
  }

  std::pair<bool,Uint32> SkinSuperToSubMesh::getSubBoneID (UintSize subMeshID, Uint32 superID)
  {
    std::pair<bool,Uint32> retval;
    Skin2MeshIter it = subs[ subMeshID ].skin2meshMap.find( superID );
    retval.first = (it != subs[ subMeshID ].skin2meshMap.end());
    retval.second = (retval.first ? it->second : 0);
    return retval;
  }

  UintSize SkinSuperToSubMesh::subMeshForFace (UintSize superGroup, UintSize superFace)
  {
    if (subMeshCount == 0) return 0;
    SkinTriMesh *super = (SkinTriMesh*) getSuperMesh();
    SkinSubMeshInfo *sub = &subs[ subMeshCount-1 ];
    UintSize newBoneCount = sub->nextBoneID;

    //Walk the face corners
    for (VertexID corner=0; corner<3; ++corner)
    {
      //Find ID of the vertex in super mesh
      VertexID superID = super->getCornerIndex( superGroup, superFace, corner );
      SkinTriMesh::Vertex *superVert = super->getVertex( superID );

      //If vertex is already in sub mesh, it can't have new bones
      if (getSubVertexID( subMeshCount-1, superID ).first == true)
        continue;

      //Check if bone index already in sub mesh
      for (int b=0; b<4; ++b) {
        if (superVert->boneWeight[b] == 0.0f) continue;
        if (getSubBoneID( subMeshCount-1, superVert->boneIndex[b] ).first == false)
          newBoneCount++;
      }
    }

    //NEW sub mesh if too many bones
    if (newBoneCount > maxBones)
      return subMeshCount;
    
    //SAME sub mesh
    return subMeshCount-1;
  }

  void SkinSuperToSubMesh::newSubVertex (UintSize subMeshID, VertexID superID)
  {
    SkinSubMeshInfo *sub = &subs[ subMeshID ];
    SkinTriMesh *super = (SkinTriMesh*) getSuperMesh();
    SkinTriMesh::Vertex *superVert = super->getVertex( superID );
    SkinTriMesh::Vertex *subVert = sub->mesh->addVertex( superVert );

    //Walk the bone indices of the vertex
    for (int b=0; b<4; ++b)
    {
      //Copy bone weight
      subVert->boneWeight[b] = superVert->boneWeight[b];

      //Skip non-weighted bones
      if (superVert->boneWeight[b] == 0.0f) {
        subVert->boneIndex[b] = 0;
        continue;
      }

      //Check if bone index already in sub mesh
      std::pair<bool,Uint32> subBoneID = getSubBoneID( subMeshID, superVert->boneIndex[b] );
      if (subBoneID.first == false)
      {
        //Map mesh to skin index and increase number of bones
        subVert->boneIndex[b] = sub->nextBoneID;
        sub->mesh->mesh2skinMap[ sub->nextBoneID ] = superVert->boneIndex[b];
        sub->skin2meshMap[ superVert->boneIndex[b] ] = sub->nextBoneID;
        sub->mesh->mesh2skinSize++;
        sub->nextBoneID++;
      }
      else
      {
        //Remap bone index from skin to group
        subVert->boneIndex[b] = subBoneID.second;
      }
    }
  }

}//namespace GE
