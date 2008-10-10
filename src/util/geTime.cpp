#include "util/geUtil.h"

namespace GE
{
  int Time::ticks = 0;
  int Time::sec = 0;
  int Time::msec = 0;
  
  void Time::ResetTicks()
  {
    #if defined(WIN32)
    Time::ticks = GetTickCount();
    #else
    struct timeval t;
    gettimeofday(&t, NULL);
    
    Time::sec = t.tv_sec;
    Time::msec = t.tv_usec;
    #endif
  }
  
  int Time::GetTicks()
  {
    #if defined(WIN32)
    return GetTickCount() - Time::ticks;
    #else
    struct timeval t;
    gettimeofday(&t, NULL);
    
    int seconds = t.tv_sec - Time::sec;
    int mseconds = (t.tv_usec / 1000);
    if (mseconds < Time::msec) seconds--;
    
    return seconds*1000 + mseconds;
    #endif
  }
}
