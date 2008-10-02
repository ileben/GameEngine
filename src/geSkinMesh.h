#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

namespace GE
{
  struct GE_API_ENTRY SkinVertex
  {
    Vector3 point;
    Uint32  boneIndex[4];
    Float32 boneWeight[4];
  };
  
  struct GE_API_ENTRY SkinFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class GE_API_ENTRY SkinMesh
  {
    DECLARE_SERIAL_CLASS (SkinMesh);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    
    DynArrayList <SkinVertex> *verts;
    DynArrayList <SkinFace> *faces;
    DynArrayList <Uint32> *indices;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&verts);
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&faces);
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&indices);
    }
    
    SkinMesh (SM *sm)
    {}
    
    SkinMesh ()
    {
      verts = new DynArrayList <SkinVertex> ();
      faces = new DynArrayList <SkinFace> ();
      indices = new DynArrayList <Uint32> ();
    }
    
    ~SkinMesh ()
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

  class SkinPose;
  class SkinAnim;
  
  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;

  public:
    
    SkinMesh *mesh;
    SkinPose *pose;
    SkinAnim *anim;
    
    void serialize (void *sm)
    {
      if (mesh != NULL)
        ((SM*)sm)->resourcePtr (&mesh);
      if (pose != NULL)
        ((SM*)sm)->resourcePtr (&pose);
      if (anim != NULL)
        ((SM*)sm)->resourcePtr (&anim);
    }
    
    MaxCharacter ()
    {
      mesh = NULL;
      pose = NULL;
      anim = NULL;
    }
    
    MaxCharacter (SM *sm) {}
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
