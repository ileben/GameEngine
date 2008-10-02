#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  struct GE_API_ENTRY SkelBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quaternion localRot;
    Vector4    localTra;
    Matrix4x4  local;
  };
  
  class GE_API_ENTRY Skeleton
  {
    DECLARE_SERIAL_CLASS (Skeleton);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    DynArrayList<SkelBone> *bones;

    Skeleton () { bones = new DynArrayList<SkelBone>; }
    ~Skeleton () { delete bones; }
    
    Skeleton (SM *sm) {}
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&bones, 1); }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
