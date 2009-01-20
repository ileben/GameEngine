#include "geObject.h"

namespace GE
{
  DEFINE_CLASS( Object );
  
  void Object::setId( const CharString &id )
  {
    this->id = id;
  }
  
  const CharString& Object::getId()
  {
    return id;
  }
}
