#ifndef __WIDGET_H
#define __WIDGET_H

#include "util/geUtil.h"
#include "engine/geVectors.h"

namespace GE
{

  class GE_API_ENTRY Widget
  {
    DECLARE_ABSTRACT (Widget); DECLARE_END;
    friend class Renderer;
    
  protected:
    Vector2 location;
    virtual void draw() = 0;
    
  public:
    virtual void setLocation(const Vector2 &l);
  };
  
}//namespace GE
#endif//__WIDGET_H
