#ifndef __GEFPSLABEL_H
#define __GEFPSLABEL_H

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
