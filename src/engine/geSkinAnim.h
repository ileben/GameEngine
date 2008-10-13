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
    DECLARE_SERIAL_CLASS( SkinTrack );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    Float32 totalTime;
    Float32 frameTime;
    ArrayList <SkinKey> keys;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->dataVar( &totalTime );
      ((SM*)sm)->dataVar( &frameTime );
      ((SM*)sm)->objectVar( &keys );
    }

    SkinTrack (SM *sm) : keys (sm)
    {}

    SkinTrack () : totalTime(0.0f), frameTime(0.0f)
    {}
    
    Quat evalAt (Float time);
  };
  
  /*
  -----------------------------------
  Animation is a set of tracks
  -----------------------------------*/
  
  class GE_API_ENTRY SkinAnim
  {
    DECLARE_SERIAL_CLASS( SkinAnim );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    CharString *name;
    ClassArrayList <SkinTrack> tracks;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->objectVar( &tracks );
    }
    
    SkinAnim (SM *sm) : tracks (sm)
    {}
    
    SkinAnim ()
    {}
    
    ~SkinAnim ()
    {
      for (UintSize t=0; t<tracks.size(); ++t)
        delete tracks[t];
    }
    
    void evalFrame (Float time);
  };
};

#endif//__GESKELANIM_H
