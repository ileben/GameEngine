#ifndef __GEOBJECT_H
#define __GEOBJECT_H

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*---------------------------------------------
   * Forward declarations
   *---------------------------------------------*/
  class Group;

  
  /*----------------------------------------------
   * Object class can be automagically saved to
   * or loaded from either text or binary format
   * saving or loading all of its properties
   *----------------------------------------------*/
  
  class GE_API_ENTRY Object
  {
    DECLARE_CLASS (Object); DECLARE_END;
    
  protected:
    
    OCC::String id;
    
  public:
    
    void setId (const OCC::String &id);
    const OCC::String& getId ();
  };

}//namespace GE
#pragma warning(pop)
#endif //__GEOBJECT_H
