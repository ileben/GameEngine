#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

namespace GE
{
  /*
  -----------------------------------
  Resource
  -----------------------------------*/
  
  struct GE_API_ENTRY SkinPolyMeshVertex
  {
    Vector3 point;
    Int32 boneIndex[4];
    Int32 weight[4];
  };
  
  struct GE_API_ENTRY SkinPolyMeshFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class GE_API_ENTRY SkinPolyMesh : public IResource
  {
  public:
    
    ArrayList_Res <SkinPolyMeshVertex, false>   verts;
    ArrayList_Res <SkinPolyMeshFace, false>     faces;
    ArrayList_Res <Int32, false>                indices;
    
    Uint32 getID() { return 2; }
    UintP getSize() { return sizeof (IResource); }
    void getPointers (SerializeManager *sm)
    {
      sm->resource (&verts);
      sm->resource (&faces);
      sm->resource (&indices);
    }
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
