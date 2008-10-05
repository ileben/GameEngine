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
      ((SM*)sm)->resourcePtr (&verts);
      ((SM*)sm)->resourcePtr (&faces);
      ((SM*)sm)->resourcePtr (&indices);
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

}//namespace GE
#endif//__GESKINPOLYMESH_H
