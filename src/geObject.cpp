#define GE_API_EXPORT
#include "geEngine.h"
using OCC::String;

namespace GE
{
  DEFINE_CLASS( Object );
  
  void Object::setId( const String &id )
  {
    this->id = id;
  }
  
  const String& Object::getId()
  {
    return id;
  }
}
