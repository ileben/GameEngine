#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

namespace GE
{
  /*
  -----------------------------------
  Resource
  -----------------------------------*/
  
  class SerializeManager;

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
  
  class GE_API_ENTRY SkinPolyMesh_Res : public IResource
  {
  public:
    
    ArrayList_Res <SkinPolyMeshVertex, false>   *verts;
    ArrayList_Res <SkinPolyMeshFace, false>     *faces;
    ArrayList_Res <Int32, false>                *indices;
        
    Uint32 getID() { return 2; }
    UintP getSize() { return sizeof (SkinPolyMesh_Res); }
    void getPointers (SerializeManager *sm)
    {
      sm->resourcePtr (&verts);
      sm->resourcePtr (&faces);
      sm->resourcePtr (&indices);
    }
    
    SkinPolyMesh_Res (SerializeManager *sm) {}

    SkinPolyMesh_Res()
    {
      verts = new ArrayList_Res <SkinPolyMeshVertex, false> ();
      faces = new ArrayList_Res <SkinPolyMeshFace, false> ();
      indices = new ArrayList_Res <Int32, false> ();
    }
    
    ~SkinPolyMesh_Res()
    {
      delete verts;
      delete faces;
      delete indices;
    }
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
