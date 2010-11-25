#ifndef __GERESOURCE_H
#define __GERESOURCE_H

#include "util/geUtil.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  class Resource : public Object
  {
    CLASS( Resource, ffc487f9,1a25,4fc6,b512e1cda7a74f46 );
    virtual void serialize( Serializer *s, Uint v )
    {
      s->string( &name );
    }
    
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
