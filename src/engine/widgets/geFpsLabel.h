#ifndef __GEFPSLABEL_H
#define __GEFPSLABEL_H

#include "util/geUtil.h"
#include "geLabel.h"

namespace GE
{

  class GE_API_ENTRY FpsLabel : public Label
  {
    DECLARE_SUBCLASS (FpsLabel, Label);
    DECLARE_END;
    
  protected:
    virtual void draw();
  };

}//namespace GE
#endif//__GEFPSLABEL_H
