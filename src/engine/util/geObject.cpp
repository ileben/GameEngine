#include "util/geUtil.h"

namespace GE
{
  Object* Class::SafeCast( const Class &to, Object *instance )
  {
    //Return early if null pointer
    if (instance == NULL)
      return NULL;

    //Return early if same class
    Class from = instance->getClass();
    if (from == to) return instance;

    //Walk up the class hierarchy until base reached
    Class prev = ClassName(Object);
    while (from != prev)
    {
      //Move one level higher
      prev = from;
      from = from->super();
      
      //Check whether [to] is [from]'s superclass
      if (from == to) return instance;
    }
    
    //Cast failed
    return NULL;
  }
}
