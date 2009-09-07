#ifndef __WIDGET_H
#define __WIDGET_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "ui/uiUI.h"

namespace GE
{

  class Widget : public Actor
  {
    DECLARE_ABSTRACT (Widget);
    DECLARE_END;
    friend class Renderer;
    
  protected:
    virtual void draw() = 0;
  };
  
}//namespace GE
#endif//__WIDGET_H
