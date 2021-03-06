#include "geFpsLabel.h"
#include "core/geGLHeaders.h"

namespace GE
{

  void FpsLabel::draw()
  {
    static int fps = -1;
    static int last = 0;
    static int drawFps = 0;
    int now = Time::GetTicks();

    if (fps == -1) {
      last = now;
      fps = 0;
    }

    ++fps;
    
    if (now - last > 1000) {
      drawFps = fps;
      last = now;
      fps = 0;
    }
    
    setText (String::Format("%d", drawFps));
    
    Label::draw ();
  }

}//namespace GE
