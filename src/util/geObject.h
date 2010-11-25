#ifndef __GEOBJECT_H
#define __GEOBJECT_H 1

namespace GE
{
  //Forward declarations
  class Serializer;
  class Object;

  /*
  -----------------------------------------------
  ClassID is a UUID
  -----------------------------------------------*/
  
  class UUID
  { public:
    Uint32 d1;
    Uint16 d2;
    Uint16 d3;
    Uint64 d4;
    
    UUID ()
      { d1=0; d2=0; d3=0; d4=0; }
    
    UUID (Uint32 dd1, Uint16 dd2, Uint16 dd3, Uint64 dd4)
      { d1=dd1; d2=dd2; d3=dd3; d4=dd4; }

    bool operator == (const UUID &c) const
      { return (d1==c.d1 && d2==c.d2 && d3==c.d3 && d4==c.d4); }
    
    bool operator == (int zero) const
      { return (d1==0 && d2==0 && d3==0 && d4==0); }

    bool operator != (int zero) const
      { return ! (operator == (0)); }
    
    bool operator < (const UUID &c) const
    {
      if (d1 < c.d1) return true;
      if (d1 > c.d1) return false;
      
      if (d2 < c.d2) return true;
      if (d2 > c.d2) return false;
      
      if (d3 < c.d3) return true;
      if (d3 > c.d3) return false;
      
      if (d4 < c.d4) return true;
      return false;
    }
  };

  /*
  ---------------------------------------------
  IClass holds the UUID and acts as a factory
  ---------------------------------------------*/

  class IClass
  {
  private:
    UUID id;
    CharString n;

  public:
    IClass (const UUID &id, const CharString &name) {
      this->id = id;
      this->n = name;
    }

    const UUID& uuid() { return id; }
    const CharString& name() { return n; }
    virtual Object* instantiate() { return NULL; }
  };

  template <class C>
  class IClass2 : public IClass
  {
  public:
    IClass2 (const UUID &id, const CharString &name) : IClass(id, name) {}
    virtual Object* instantiate() { return new C(); }
  };

  /*
  ----------------------------------------------
  Class is a wrapper for IClass pointer which
  implements comparison by UUID.
  ----------------------------------------------*/

  class Class
  {
    IClass *ptr;

  public:

    Class () {
      ptr = NULL;
    }

    Class (IClass *ptr) {
      this->ptr = ptr;
    }

    void operator= (IClass *ptr) {
      this->ptr = ptr;
    }

    bool operator== (const Class &other) const {
      return ptr->uuid() == other.ptr->uuid();
    }

    bool operator!= (const Class &other) const {
      return !operator==( other );
    }

    bool operator< (const Class &other) const {
      return ptr->uuid() < other.ptr->uuid();
    }

    IClass* operator-> () {
      return ptr;
    }

    IClass& operator* () {
      return *ptr;
    }
  };

  /*
  ------------------------------------------------------------
  Factory uses type dispatching to instantiate objects using
  either the default constructor or custom one if specified.
  ------------------------------------------------------------*/

  class IFactory
  {
  public:
    virtual Object* produce() = 0;
  };

  template <class C>
  class Factory : public IFactory
  {
  public:

    //This uses the return type of produce function to determine
    //whether a custom one has been defined for this class
    virtual Object* produce() {
      return produceDispatch( &C::produce );
    };

  private:

    //This will match produce function with correct return type
    Object* produceDispatch (C* (*funcPtr) ()) {
      return C::produce();
    }

    //This will match produce function with any other return type
    template <class Other>
    Object* produceDispatch (Other* (*funcPtr) ()) {
      return new C();
    }
  };

  /*
  ---------------------------------------------
  Base Object class for serialization
  ---------------------------------------------*/

  class Object
  {
    friend class Serializer;

  private:
    UintSize serialID;

  public:
    virtual Class getClass() = 0;
    virtual Uint version () { return 1; }
    virtual void serialize (Serializer *serializer, Uint version) {}
    static Object* produce() { return NULL; }
  };

  /*
  ----------------------------------------------------------
  Helper macros for easier object definition
  ----------------------------------------------------------*/

#define MAKEUUID( A,B,C,D ) \
  UUID( 0x ## A ## u, 0x ## B ## u, 0x ## C ## u, 0x ## D ## ull)


#define __CLASS( Interface, Name, A,B,C,D ) \
  public: \
  \
  static Class GetClass() { \
  static Interface c( MAKEUUID(A,B,C,D), #Name); \
    return Class( &c ); } \
    \
  virtual Class getClass() { \
    return Name::GetClass(); }


#define __TEMPLATE_CLASS( Interface, Name, A,B,C,D ) \
  template <> Class Name::GetClass() { \
    static Interface c( MAKEUUID(A,B,C,D), #Name ); \
    return Class( &c ); }


#define CLASS( Name, A,B,C,D ) \
  __CLASS( IClass2<Name>, Name, A,B,C,D )

#define ABSTRACT( Name, A,B,C,D ) \
  __CLASS( IClass, Name, A,B,C,D );

#define TEMPLATE_CLASS( Name, A,B,C,D ) \
  __TEMPLATE_CLASS( IClass2<Name>, Name, A,B,C,D )


}//namespace GE

using GE::UUID;

#endif//__GEOBJECT_H
