#include "geWidget.h"

namespace GE
{
  DEFINE_CLASS (Widget);
  
  void Widget::setLocation(const Vector2 &l) {
    location = l;
  }

}//namespace GE
