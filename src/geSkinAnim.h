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
    Quat value;
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
    Float32 totalTime;
    Float32 frameTime;
    ArrayListT <SkinKey> *keys;
    
    SkinTrack (SM *sm) {}

    void serialize (void *sm)
    {
      ((SM*)sm)->memberVar (&totalTime);
      ((SM*)sm)->memberVar (&frameTime);
      ((SM*)sm)->resourcePtr (&keys);
    }

    SkinTrack ()
    {
      totalTime = 0.0f;
      frameTime = 0.0f;
      keys = new ArrayListT <SkinKey>;
    }

    ~SkinTrack () { delete keys; }
    
    Quat evalAt (Float time);
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
    OCC::CharString *name;
    ClassArrayList <SkinTrack> *tracks;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->resourcePtr (&tracks);
    }
    
    SkinAnim (SM *sm) {}
    
    SkinAnim ()
    {
      tracks = new ClassArrayList <SkinTrack>;
    }
    
    ~SkinAnim ()
    {
      for (int t=0; t<tracks->size(); ++t)
        delete tracks->at (t);

      delete tracks;
    }

    void evalFrame (Float time);
  };
};

#endif//__GESKELANIM_H
