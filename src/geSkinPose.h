#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  struct GE_API_ENTRY SkinBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quat       localRot;
    Vector4    localTra;
  };
  
  class GE_API_ENTRY SkinPose
  {
    DECLARE_SERIAL_CLASS (SkinPose);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    DynArrayList <SkinBone> *bones;
    
    SkinPose (SM *sm) {}
    SkinPose () { bones = new DynArrayList <SkinBone>; }
    ~SkinPose () { delete bones; }
    void serialize (void *sm) { ((SM*)sm)->resourcePtr (&bones); }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
