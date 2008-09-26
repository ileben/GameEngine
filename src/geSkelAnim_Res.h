#ifndef __GESKELANIM_H
#define __GESKELANIM_H

namespace GE
{
  /*
  -----------------------------------
  Resource
  -----------------------------------*/

  struct SkelAnimKey
  {
    Float32     time;
    Quaternion  value;
  };

  struct SkelAnimTrack
  {
    Int32         numKeys;
    SkelAnimKey  *keys;
  };
  
  class GE_API_ENTRY SkelAnim_Res
  {
    friend class SkelAnim_Factory;

  private:
    Float32         duration;
    Int32           numTracks;
    SkelAnimTrack  *tracks;

  public:
    SkelAnim_Res ();
  };

  /*
  -----------------------------------
  Factory
  -----------------------------------*/
  
  class GE_API_ENTRY SkelAnimTrack_Factory
  {
  public:
    OCC::ArrayList <SkelAnimKey*>  keys;
  };

  class GE_API_ENTRY SkelAnim_Factory
  {
  public:
    Float32                                  duration;
    OCC::ArrayList <SkelAnimTrack_Factory*>  tracks;

    void create (void **outMem, UintP *outSize);
  };
  
};

#endif//__GESKELANIM_H
