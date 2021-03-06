#ifndef __GEPROPERTY_H
#define __GEPROPERTY_H

#include "util/geUtil.h"

namespace GE
{
  using namespace TextParserCommon;
  
  class Object;
  
  /*
  The static function allows retreival of default value via
  class name. The non static function is virtual so retreival
  via object instance pointer returns the default value of its
  final class.
  */
  #define DEFAULT( type, name, value ) \
  static inline type __static_default_ ## name () \
    { return value; } \
  virtual type __default_ ## name () \
    { return __static_default_ ## name (); }
  
  /*
  Getter functions return default values
  */
  #define GetClassDefault( Class, name ) \
  Class::__static_default_ ## name ()
    
  #define GetDefault( instance, name ) \
  (instance)->__default_ ## name ()
  
  /*
  Abstract property prototype
  */
  class Property
  {
    friend class Object;
    
  protected:
    CharString name;
    
  public:
    Property () {};
    Property (const char *newName) { name = newName; };
    
    const CharString& getName ();
    virtual void saveText (void *obj, ByteString &buf) = 0;
    virtual void saveBinary (void *obj, ByteString &buf) = 0;
    virtual int loadText (void *obj, const ByteString &buf, int index) = 0;
    virtual int loadBinary (void *obj, const ByteString &buf, int index) = 0;
  };
  
  /*
  Templated prototype subclass that handles basic types
  */
  
  template <class T, class C>
  class TypeProperty : public Property
  {
  private:
    T C::*pvalue;
    
  public:
    TypeProperty (T C::*pnewValue, const char* newName)
      : Property (newName) { pvalue = pnewValue; }
    
    /*
    Overloads of private __saveText function handle saving of
    various basic types
    */
    
  private:
    
    template <class V>
    void __saveText (C *obj, V C::*val, ByteString &buf) {}
    
    void __saveText (C *obj, int C::*val, ByteString &buf)
      { buf += ByteString::FromInteger (obj->*pvalue); }
    
    void __saveText (C *obj, float C::*val, ByteString &buf)
      { buf += ByteString::FromFloat (obj->*pvalue); }
    
    void __saveText (C *obj, String C::*val, ByteString &buf)
      { buf += obj->*pvalue; }
    
    /*
    Public saving functions resolve to proper private overloads
    efficiently at compile time
    */
    
  public:
    
    virtual void saveText (void *obj, ByteString &buf)
      { __saveText ((C*) obj, pvalue, buf); }
    
    virtual void saveBinary (void *obj, ByteString &buf)
      {}
    
    
    /*
    Overloads of private loading function handle loading of
    various basic types
    */
    
  private:
    
    template <class V>
    void __loadText (C *obj, V C::*val, const ByteString &buf, int index) {}
    
    int __loadText (C *obj, int C::*val, const ByteString &buf, int index)
    {
      int len = 0;
      obj->*pvalue = buf.parseIntegerAt (index, &len);
      return index + len;
    }
    
    int __loadText (C *obj, float C::*val, const ByteString &buf, int index)
    {
      int len = 0;
      obj->*pvalue = buf.parseFloatAt (index, &len);
      return index + len;
    }
    
    int __loadText (C *obj, String *val, const ByteString &buf, int index)
    {
      TextParser<ByteString> parser (&buf, index);
      
      parser.jump <Or <Char <'"'>, S > > (false);
      parser.fw (1);
      parser.beginToken ();
      
      parser.jump <Or <Char <'"'>, S > > (false);
      parser.endToken ();
      
      obj->*pvalue = parser.getToken ();
      parser.fw (1);
      
      return parser.getIndex();
    }
    
    /*
    Public loading functions resolve to proper private overloads
    efficiently at compile time
    */
    
  public:
    
    virtual int loadText (void *obj, const ByteString &buf, int index)
      { return __loadText ((C*) obj, pvalue, buf, index); }
    
    virtual int loadBinary (void *obj, const ByteString &buf, int index)
      { return index; }
  };

}//namespace GE
#endif//__GEPROPERTY_H
