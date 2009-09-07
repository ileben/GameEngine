#ifndef __GELABEL_H
#define __GELABEL_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "geWidget.h"

#pragma warning(push)
#pragma warning(disable:4251)


namespace GE
{
  class GE_API_ENTRY Label : public Widget
  {
    DECLARE_SUBCLASS (Label, Widget);
    DECLARE_END;
    
  protected:
    String text;
    Vector3 color;
    virtual void draw();
    
  public:
    void setText (const String &text);
    void setColor (const Vector3 &c);
    Vector3 getColor () { return color; }
  };

}//namespace GE
#pragma warning(pop)
#endif//__GELABEL_H
