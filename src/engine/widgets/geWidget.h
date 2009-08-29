#ifndef __WIDGET_H
#define __WIDGET_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "ui/uiUI.h"

namespace GE
{

  class UIWidget : public UI::Widget
  {
    DECLARE_ABSTRACT (UIWidget);
    DECLARE_END;
    friend class Renderer;
    
  protected:
    virtual void draw() = 0;
  };
  
}//namespace GE
#endif//__WIDGET_H
