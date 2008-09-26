#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

namespace GE
{
  /*
  -----------------------------------
  Resource
  -----------------------------------*/

  struct GE_API_ENTRY SkeletonBone
  {
    Int32      numChildren;
    Matrix4x4  poseInverse;
  };
  
  class GE_API_ENTRY Skeleton_Res
  {
    friend class  Skeleton_Factory;
    
  private:
    Int32          numBones;
    SkeletonBone  *bones;

  public:
    Skeleton_Res ();
  };
  
  /*
  -----------------------------------
  Factory
  -----------------------------------*/

  class GE_API_ENTRY Skeleton_Factory
  {
  public:
    OCC::ArrayList<SkeletonBone*> bones;

    void create (void **outMem, UintP *outSize);
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
