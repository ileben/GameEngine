#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  struct GE_API_ENTRY SkinBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quat       localR;
    Vector4    localT;
    Matrix4x4  localS;
  };
  
  class GE_API_ENTRY SkinPose
  {
    DECLARE_SERIAL_CLASS (SkinPose);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    ArrayListT <SkinBone> *bones;
    
    SkinPose (SM *sm) {}
    SkinPose () { bones = new ArrayListT <SkinBone>; }
    ~SkinPose () { delete bones; }
    void serialize (void *sm) { ((SM*)sm)->resourcePtr (&bones); }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
