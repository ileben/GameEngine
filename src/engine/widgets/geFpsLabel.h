#ifndef __GEFPSLABEL_H
#define __GEFPSLABEL_H

#include "util/geUtil.h"
#include "geLabel.h"

namespace GE
{

  class FpsLabel : public Label
  {
    CLASS( FpsLabel, Label,
      533bfe43,3fc2,4baa,bb36d7b791924291 );
    
  protected:
    virtual void draw();
  };

}//namespace GE
#endif//__GEFPSLABEL_H
