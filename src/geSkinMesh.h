#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

namespace GE
{
  struct GE_API_ENTRY SkinPolyVertex
  {
    Vector3 point;
    Uint32  boneIndex[4];
    Float32 boneWeight[4];
  };
  
  struct GE_API_ENTRY SkinPolyFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class GE_API_ENTRY SkinPolyMesh_Res
  {
    DECLARE_SERIAL_CLASS (SkinPolyMesh_Res);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    
    DynArrayList <SkinPolyVertex> *verts;
    DynArrayList <SkinPolyFace> *faces;
    DynArrayList <Uint32> *indices;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&verts, 1);
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&faces, 1);
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&indices, 1);
    }
    
    SkinPolyMesh_Res (SM *sm)
    {}
    
    SkinPolyMesh_Res()
    {
      verts = new DynArrayList <SkinPolyVertex> ();
      faces = new DynArrayList <SkinPolyFace> ();
      indices = new DynArrayList <Uint32> ();
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

  class Skeleton;
  class SkelAnim;
  
  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;

  public:
    
    SkinPolyMesh_Res *mesh;
    Skeleton *skeleton;
    SkelAnim *animation;
    
    void serialize (void *sm)
    {
      if (mesh != NULL)
        ((SM*)sm)->resourcePtr (&mesh);
      if (skeleton != NULL)
        ((SM*)sm)->resourcePtr (&skeleton);
      if (animation != NULL)
        ((SM*)sm)->resourcePtr (&animation);
    }
    
    MaxCharacter ()
    {
      mesh = NULL;
      skeleton = NULL;
      animation = NULL;
    }
    
    MaxCharacter (SM *sm) {}
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
