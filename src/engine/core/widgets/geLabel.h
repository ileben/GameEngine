#ifndef __GELABEL_H
#define __GELABEL_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "geWidget.h"

#pragma warning(push)
#pragma warning(disable:4251)


namespace GE
{
  class Label : public Widget
  {
    CLASS( Label, Widget,
      3e6873eb,9602,4838,b4bbe297cca8ee01 );
    
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
