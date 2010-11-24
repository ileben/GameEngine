#ifndef __GEOBJECT_H
#define __GEOBJECT_H 1

namespace GE
{
  //Forward declarations
  class Serializer;
  
  /*
  ---------------------------------------------
  Base Object class for serialization
  ---------------------------------------------*/

  class MyObject
  {
    friend class Serializer;

  private:
    UintSize serialID;

  public:
    virtual ClassID uuid () = 0;
    virtual Uint version () { return 1; }
    virtual void serialize (Serializer *serializer, Uint version) = 0;
  };

  /*
  ---------------------------------------------
  Factory instantiates objects
  ---------------------------------------------*/

  class BaseFactory
  {
  public:
    virtual MyObject* produce() = 0;
  };

  template <class C>
  class Factory : public BaseFactory
  {
  public:
    virtual MyObject* produce()
    {
      return new C();
    }
  };

  /*
  --------------------------------------------------------------
  ClassFactory registers objects by UUID for deserialization
  --------------------------------------------------------------*/

  class ClassFactory
  {
    static std::map< ClassID, BaseFactory* > factories;

  public:

    template <class C> static void Register ()
    {
      factories[ C::Uuid() ] = new Factory<C>;
    }

    static MyObject* Produce (const ClassID &id)
    {
      return factories[ id ]->produce();
    }
  };

  /*
  ----------------------------------------------------------
  Helper macros for easier object definition
  ----------------------------------------------------------*/

  #define UUID( a,b,c,d ) \
    public: \
    static ClassID Uuid() { return ClassID( a, b, c, d ); } \
    virtual ClassID uuid() { return ClassID( a, b, c, d ); }


}//namespace GE
#endif//__GEOBJECT_H
