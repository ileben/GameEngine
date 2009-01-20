#ifndef __GERESOURCE_H
#define __GERESOURCE_H

#include "util/geUtil.h"
#include "geObject.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  class GE_API_ENTRY Resource : public Object
  {
    DECLARE_SUBCLASS (Resource, Object); DECLARE_END;
    
  protected:
    int refcount;
    
  public:
    Resource();
    int reference();
    int dereference();
    int getRefCount();
  };
  
}//namespace GE
#pragma warning(pop)
#endif//__GERESOURCE_H
