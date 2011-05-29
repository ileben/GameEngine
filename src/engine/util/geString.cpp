#include "util/geUtil.h"

namespace GE
{ 
  
  #if defined(WIN32)

  int win_snprintf(char *str, size_t size, const char *format, ...)
  {
    size_t count;
    va_list ap;
    va_start(ap, format);
    count = _vscprintf(format, ap);
    _vsnprintf(str, size, format, ap);
    va_end(ap);
    return (int)count;
  }

  int win_vsnprintf(char *str, size_t size, const char *format, va_list ap)
  {
    size_t count;
    count = _vscprintf(format, ap);
    _vsnprintf(str, size, format, ap);
    return (int)count;
  }

  #endif//WIN32


}//namespace GE
