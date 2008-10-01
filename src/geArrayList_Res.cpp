#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{
  typedef ArrayList_Res <Int32> ArrayList_Res_I32;
  
  template <> DEFINE_SERIAL_CLASS (ArrayList_Res_I32,  CLSID_ARRAYLIST_RES_I32);
}
