#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  struct GE_API_ENTRY SkeletonBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quaternion localRot;
    Vector4    localTra;
    Matrix4x4  local;
  };
  
  typedef ArrayList_Res <SkeletonBone,false> ArrayList_Res_SB;
  
  class GE_API_ENTRY Skeleton_Res : public IResource
  {
    DECLARE_SERIAL_CLASS (Skeleton_Res);
    DECLARE_END;
    
  public:
    ArrayList_Res_SB *bones;

    Skeleton_Res () { bones = new ArrayList_Res_SB; }
    ~Skeleton_Res () { delete bones; }
    
    Skeleton_Res (SerializeManager *sm) {}
    void getPointers (SerializeManager *sm) { sm->resourcePtr (&bones); }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
