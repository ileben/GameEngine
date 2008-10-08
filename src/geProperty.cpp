#define GE_API_EXPORT
#include "geEngine.h"
using OCC::CharString;

namespace GE
{
  const CharString& Property::getName ()
  {
    return name;
  }
}
