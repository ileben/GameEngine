#include "geResource.h"

namespace GE
{
  Resource::Resource() {
    refcount = 0;
  }

  int Resource::reference() {
    return ++refcount;
  }

  int Resource::dereference() {
    if (refcount == 0) return 0;
    return --refcount;
  }

  int Resource::getRefCount() {
    return refcount;
  }

  void Resource::setResourceName (const CharString &n)
  {
    name = n;
  }
}
