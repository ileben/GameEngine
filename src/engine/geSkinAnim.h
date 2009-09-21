#ifndef __GESKELANIM_H
#define __GESKELANIM_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "engine/geAnimation.h"

namespace GE
{

  /*
  -----------------------------------
  Animation is a set of tracks
  -----------------------------------*/
  
  class SkinAnim : public Object
  {
    DECLARE_SERIAL_SUBCLASS( SkinAnim, Object );
    DECLARE_OBJVAR( name );
    DECLARE_DATAVAR( duration );
    DECLARE_OBJVAR( tracksT );
    DECLARE_OBJVAR( tracksR );
    DECLARE_END;
    
  public:
    CharString name;
    Float duration;
    ObjPtrArrayList <Vec3AnimTrack> tracksT;
    ObjPtrArrayList <QuatAnimTrack> tracksR;
    
    SkinAnim (SM *sm) : name (sm), tracksT (sm), tracksR(sm) {}
    SkinAnim () {}
    ~SkinAnim ()
    {
      for (UintSize t=0; t<tracksT.size(); ++t)
        delete tracksT[t];
      for (UintSize t=0; t<tracksR.size(); ++t)
        delete tracksR[t];
    }
    
    void evalFrame (Float time);
  };
};

#endif//__GESKELANIM_H
