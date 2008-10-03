#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  struct GE_API_ENTRY SkinBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quaternion localRot;
    Vector4    localTra;
    Matrix4x4  local;
  };
  
  class GE_API_ENTRY SkinPose
  {
    DECLARE_SERIAL_CLASS (SkinPose);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    DynArrayList<SkinBone> *bones;

    SkinPose () { bones = new DynArrayList<SkinBone>; }
    ~SkinPose () { delete bones; }
    
    SkinPose (SM *sm) {}
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (&bones); }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
