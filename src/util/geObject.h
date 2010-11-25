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

#if (1)
  /*
  ---------------------------------------------
  Class holds the UUID and acts as a factory
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
    virtual Object* instantiate() = 0;
  };

  template <class C>
  class IClass2 : public IClass
  {
  public:
    IClass2 (const UUID &id, const CharString &name) : IClass(id, name) {}
    virtual Object* instantiate() { return new C(); }
  };

  /*
  ---------------------------------------------------------
  ClassPtr is a comparable wrapper for class based on UUID
  ---------------------------------------------------------*/

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

#else

  class FactoryBase
  {
  public:
    virtual Object* produce() = 0;
  };

  template <class C>
  class Factory
  {
  public:
    virtual Object* produce() {
      return new C();
    }
  };

#endif

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
    //virtual UUID uuid() = 0;
    virtual Class getClass() = 0;
    virtual Uint version () { return 1; }
    virtual void serialize (Serializer *serializer, Uint version) {}
  };

  /*
  ----------------------------------------------------------
  Helper macros for easier object definition
  ----------------------------------------------------------*/

#if(1)

  #define CLASS( Name, A,B,C,D ) \
    public: \
    \
    static Class GetClass() { \
      static IClass2<Name> c( UUID( 0xAu, 0xBu, 0xCu, 0xDull), #Name); \
      return Class( &c ); } \
      \
    virtual Class getClass() { \
      return Name::GetClass(); }
#else
  
  #define CLASSID( a,b,c,d ) \
    public: \
    static UUID Uuid() { return UUID( a, b, c, d ); } \
    virtual UUID uuid() { return UUID( a, b, c, d ); }

#endif

}//namespace GE
#endif//__GEOBJECT_H
