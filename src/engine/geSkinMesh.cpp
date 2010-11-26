#include "geSkinMesh.h"

namespace GE
{

  void SkinTriMesh::fromPoly (PolyMesh *m, TexMesh *um)
  {
    binding.init( getFormat() );
    TriMesh::fromPoly( m, um );
  }

  void SkinTriMesh::vertexFromPoly (PolyMesh::Vertex *polyVert,
                                    PolyMesh::HalfEdge *polyHedge,
                                    TexMesh::Vertex *texVert)
  {
    TriMesh::vertexFromPoly( polyVert, polyHedge, texVert );

    SPolyMesh::Vertex *skinVert = (SPolyMesh::Vertex*) polyVert;
    SkinVertex triVert =  binding( getVertex( getVertexCount()-1 ) );

    for (int i=0; i<4; ++i)
      triVert.jointIndex[ i ] = skinVert->boneIndex[ i ];
    
    for (int i=0; i<4; ++i)
      triVert.jointWeight[ i ] = skinVert->boneWeight[ i ];
  }

  /*
  -------------------------------------------------------------
  Algorithm that copies faces of a mesh into a sub mesh and
  remaps the joint indices until the joint limit is reached
  -------------------------------------------------------------*/

  void SkinSuperToSubMesh::splitByBoneLimit (Uint32 maxBonesPerMesh)
  {
    maxBones = maxBonesPerMesh;
    subMeshCount = 0;
    binding.init( getSuperMesh()->getFormat() );
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
      SkinVertex superVert = binding( super->getVertex( superID ) );

      //If vertex is already in sub mesh, it can't have new bones
      if (getSubVertexID( subMeshCount-1, superID ).first == true)
        continue;

      //Check if joint index already in sub mesh
      for (int j=0; j<4; ++j) {
        if (superVert.jointWeight[j] == 0.0f) continue;
        if (getSubBoneID( subMeshCount-1, superVert.jointIndex[j] ).first == false)
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
    
    void *superData = super->getVertex( superID );
    void *subData = sub->mesh->addVertex( superData );

    SkinVertex superVert = binding( superData );
    SkinVertex subVert = binding( subData );

    //Walk the joint indices of the vertex
    for (int j=0; j<4; ++j)
    {
      //Copy joint weight
      subVert.jointWeight[j] = superVert.jointWeight[j];

      //Skip non-weighted joints
      if (superVert.jointWeight[j] == 0.0f) {
        subVert.jointIndex[j] = 0;
        continue;
      }

      //Check if bone index already in sub mesh
      std::pair<bool,Uint32> subBoneID = getSubBoneID( subMeshID, superVert.jointIndex[j] );
      if (subBoneID.first == false)
      {
        //Map mesh to skin index and increase number of joints
        subVert.jointIndex[j] = sub->nextBoneID;
        sub->mesh->mesh2skinMap[ sub->nextBoneID ] = superVert.jointIndex[j];
        sub->skin2meshMap[ superVert.jointIndex[j] ] = sub->nextBoneID;
        sub->mesh->mesh2skinSize++;
        sub->nextBoneID++;
      }
      else
      {
        //Remap joint index from skin to group
        subVert.jointIndex[j] = subBoneID.second;
      }
    }
  }

}//namespace GE
