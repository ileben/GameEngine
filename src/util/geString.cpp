#include "util/geUtil.h"

namespace GE
{

  DEFINE_SERIAL_TEMPL_CLASS( BasicString<char>,     CLSID_CHARSTRING );
  DEFINE_SERIAL_TEMPL_CLASS( BasicString<Byte>,     CLSID_BYTESTRING );
  DEFINE_SERIAL_TEMPL_CLASS( BasicString<Unicode>,  CLSID_UNICODESTRING );
  
  
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
