#ifndef __GETIME_H
#define __GETIME_H

namespace GE
{
  class Time
  {
  private:
    static int ticks;
    static int sec;
    static int msec;

  public:
		static void ResetTicks();
		static int GetTicks();
  };
};

#endif//__GETIME_H
