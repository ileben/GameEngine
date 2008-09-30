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
    Uint32  boneIndex[4];
    Float32 boneWeight[4];
  };
  
  struct GE_API_ENTRY SkinPolyMeshFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class GE_API_ENTRY SkinPolyMesh_Res : public IResource
  {
    DECLARE_SERIAL_CLASS (SkinPolyMesh_Res);
    DECLARE_END;
    
  public:
    
    ArrayList_Res <SkinPolyMeshVertex, false>   *verts;
    ArrayList_Res <SkinPolyMeshFace, false>     *faces;
    ArrayList_Res <Int32, false>                *indices;
    
    void getPointers (SerializeManager *sm)
    {
      sm->resourcePtr (&verts);
      sm->resourcePtr (&faces);
      sm->resourcePtr (&indices);
    }
    
    SkinPolyMesh_Res (SerializeManager *sm)
    {}

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

  /*
  -----------------------------------
  Resource
  -----------------------------------*/

  class Skeleton_Res;
  
  class GE_API_ENTRY MaxCharacter_Res : public IResource
  {
    DECLARE_SERIAL_CLASS (MaxCharacter_Res)
    DECLARE_END;

  public:
    
    SkinPolyMesh_Res *mesh;
    Skeleton_Res *skeleton;

    void getPointers (SerializeManager *sm)
    {
      if (mesh != NULL)
        sm->resourcePtr (&mesh);
      if (skeleton != NULL)
        sm->resourcePtr (&skeleton);
    }
    
    MaxCharacter_Res ()
    {
      mesh = NULL;
      skeleton = NULL;
    }
    
    MaxCharacter_Res (SerializeManager *sm) {}
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
