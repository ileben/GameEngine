#ifndef __GEOBJECT_H
#define __GEOBJECT_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*----------------------------------------------
   * Object class can be automagically saved to
   * or loaded from either text or binary format
   * saving or loading all of its properties
   *----------------------------------------------*/
  
  class GE_API_ENTRY Object
  {
    DECLARE_CLASS (Object); DECLARE_END;
    
  protected:
    
    CharString id;
    
  public:
    
    void setId (const CharString &id);
    const CharString& getId ();
  };

}//namespace GE
#pragma warning(pop)
#endif //__GEOBJECT_H
