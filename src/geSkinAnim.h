#ifndef __GESKELANIM_H
#define __GESKELANIM_H

namespace GE
{
  /*
  -----------------------------------
  Animation key
  -----------------------------------*/

  struct SkinKey
  {
    Float32     time;
    Quaternion  value;
  };

  /*
  -----------------------------------
  Track is a set of keys
  -----------------------------------*/
  
  class GE_API_ENTRY SkinTrack
  {
    DECLARE_SERIAL_CLASS (SkinTrack);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;
    
  public:
    DynArrayList <SkinKey> *keys;
    
    SkinTrack (SM *sm) {}
    SkinTrack () { keys = new DynArrayList <SkinKey>; }
    ~SkinTrack () { delete keys; }
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&keys); }
  };

  /*
  -----------------------------------
  Animation is a set of tracks
  -----------------------------------*/
  
  class GE_API_ENTRY SkinAnim
  {
    DECLARE_SERIAL_CLASS (SkinAnim);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;

  public:
    Float32 duration;
    ResPtrArrayList <SkinTrack*> *tracks;
    
    SkinAnim (SM *sm) {}
    SkinAnim () { tracks = new ResPtrArrayList <SkinTrack*>; }
    ~SkinAnim () { delete tracks; }
    void serialize (void *sm) {
      ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&tracks); }
  };
};

#endif//__GESKELANIM_H
