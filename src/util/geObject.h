#ifndef __GEOBJECT_H
#define __GEOBJECT_H 1

namespace GE
{
  //Forward declarations
  class Serializer;
  class Object;
  class IClass;
  class Class;

  /*
  ------------------------------------------------------------------
  UUID is used to identify classes. It is safter than built-in RTTI,
  because in some implementations typeid() == typeid() does pointer
  comparison which breaks in DLLs. It is faster than built-in RTTI,
  because in some implementations typeid() uses string comparison.
  ------------------------------------------------------------------*/
  
  struct UUID
  {
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
  IClass level-1 provides UUID and name
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

    const UUID& uuid() const { return id; }
    const CharString& name() const { return n; }
    
    virtual Class super() const = 0;
    virtual Object* instantiate() const = 0;
  };

  /*
  ----------------------------------------------------
  Class is a wrapper for IClass pointer which forces
  comparison by UUID and implements safe casting.
  ----------------------------------------------------*/

  class Class
  {
    friend class IClass;
    IClass *ptr;

  public:

    Class () {
      ptr = NULL;
    }

    Class (IClass *ptr) {
      this->ptr = ptr;
    }

    void operator= (const IClass *ptr) {
      this->ptr = const_cast< IClass* >(ptr);
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

    const IClass* operator-> () const {
      return ptr;
    }

    const IClass& operator* () const {
      return *ptr;
    }

    static Object* SafeCast( const Class &to, Object *instance );

    template <class T>
    static T* SafeCast( Object *instance ) {
      return (T*) SafeCast( T::GetClass(), instance );
    }
  };

  /*
  -----------------------------------------------
  IClass level-2 provides access to super class
  -----------------------------------------------*/

  template <class ThisClass, class SuperClass>
  class IClass2 : public IClass
  {
  public:
    IClass2 (const UUID &id, const CharString &name) : IClass(id,name) {}
    virtual Class super() const { return SuperClass::GetClass(); }
  };

  /*
  ---------------------------------------------
  IClass level-3 provides UUID and name
  ---------------------------------------------*/

  template <class ThisClass, class SuperClass>
  class IConcrete: public IClass2< ThisClass, SuperClass >
  {
  public:
    IConcrete (const UUID &id, const CharString &name) : IClass2(id,name) {}
    virtual Object* instantiate() const { return new ThisClass(); }
  };

  template <class ThisClass, class SuperClass>
  class IAbstract: public IClass2< ThisClass, SuperClass >
  {
  public:
    IAbstract (const UUID &id, const CharString &name) : IClass2(id,name) {}
    virtual Object* instantiate() const { return NULL; }
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
  ----------------------------------------------------------
  Helper macros for object definition
  ----------------------------------------------------------*/

#define MAKEUUID( A,B,C,D ) \
  UUID( 0x ## A ## u, 0x ## B ## u, 0x ## C ## u, 0x ## D ## ull)


#define __CLASS( Interface, Name, Super, A,B,C,D ) \
  public: \
  \
  static Class GetClass() { \
    static Interface< Name, Super > c( MAKEUUID(A,B,C,D), #Name); \
    return Class( &c ); } \
    \
  virtual Class getClass() { \
    return Name::GetClass(); }


#define __TEMPLATE_CLASS( Interface, Name, Super, A,B,C,D ) \
  template <> Class Name::GetClass() { \
    static Interface< Name, Super > c( MAKEUUID(A,B,C,D), #Name ); \
    return Class( &c ); }


#define CLASS( Name, Super, A,B,C,D ) \
  __CLASS( IConcrete, Name, Super, A,B,C,D )

#define ABSTRACT( Name, Super, A,B,C,D ) \
  __CLASS( IAbstract, Name, Super, A,B,C,D );

#define TEMPLATE_CLASS( Name, Super, A,B,C,D ) \
  __TEMPLATE_CLASS( IConcrete, Name, Super, A,B,C,D )

  /*
  ----------------------------------------------------------
  Helper macros for access of Class
  ----------------------------------------------------------*/

#define ClassOf( o ) o->getClass()
#define ClassName( C ) C::GetClass()


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
    ABSTRACT( Object, Object, 0,0,0,0 );

    virtual Uint version () { return 1; }
    static Object* produce() { return NULL; }
    virtual void serialize (Serializer *serializer, Uint version) {}
  };

}//namespace GE

using GE::UUID;

#endif//__GEOBJECT_H
