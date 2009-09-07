#ifndef __GERESOURCE_H
#define __GERESOURCE_H

#include "util/geUtil.h"
#include "geObject.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  class Resource
  {
    DECLARE_CLASS( Resource );
    DECLARE_OBJVAR( name );
    DECLARE_END;
    
  protected:
    CharString name;
    int refcount;
    
  public:
    Resource ();

    int reference();
    int dereference();
    int getRefCount();

    const CharString& getResourceName() { return name; }
    void setResourceName (const CharString &name);
  };
  
}//namespace GE
#pragma warning(pop)
#endif//__GERESOURCE_H
