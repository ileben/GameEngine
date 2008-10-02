#ifndef __GESKELANIM_H
#define __GESKELANIM_H

namespace GE
{
  /*
  -----------------------------------
  Animation key
  -----------------------------------*/

  struct SkelKey
  {
    Float32     time;
    Quaternion  value;
  };

  /*
  -----------------------------------
  Track is a set of keys
  -----------------------------------*/
  
  class GE_API_ENTRY SkelTrack
  {
    DECLARE_SERIAL_CLASS (SkelTrack);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    DynArrayList <SkelKey> *keys;
    
    SkelTrack (SM *sm) {}
    SkelTrack () { keys = new DynArrayList <SkelKey>; }
    ~SkelTrack () { delete keys; }
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&keys, 1); }
  };

  /*
  -----------------------------------
  Animation is a set of tracks
  -----------------------------------*/
  
  class GE_API_ENTRY SkelAnim
  {
    DECLARE_SERIAL_CLASS (SkelAnim);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;

  public:
    Float32 duration;
    ResPtrArrayList <SkelTrack*> *tracks;
    
    SkelAnim (SM *sm) {}
    SkelAnim () { tracks = new ResPtrArrayList <SkelTrack*>; }
    ~SkelAnim () { delete tracks; }
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&tracks, 1); }
  };
};

#endif//__GESKELANIM_H
