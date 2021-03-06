#pragma warning(push)
#pragma warning(disable:4251)

/*
----------------------------------------------------------------
DECLARE_DLL_SPEC should be #defined to a keyword specifiying
dll export or import action for the current declaration!
----------------------------------------------------------------*/

#ifndef CLASS_DLL_ACTION
#  define CLASS_DLL_ACTION
#endif



/*
-------------------------------------------------------------
The rest can only be #included once.
-------------------------------------------------------------*/

#ifndef __GECLASS_H
#define __GECLASS_H

#include <map>
#include <string>

namespace GE
{
  /*
  -----------------------------------------------
  Forward declarations
  -----------------------------------------------*/

  class IMember;
  class IClass;
  class ObjectOld;
  class Property;
  class SerializeManager;
  
  /*
  -----------------------------------------------
  ClassID is a UUID
  -----------------------------------------------*/
  
  class ClassID
  { public:
    Uint32 d1;
    Uint16 d2;
    Uint16 d3;
    Uint64 d4;
    
    ClassID (SerializeManager *sm) {}
    
    ClassID ()
      { d1=0; d2=0; d3=0; d4=0; }
    
    ClassID (Uint32 dd1, Uint16 dd2, Uint16 dd3, Uint64 dd4)
      { d1=dd1; d2=dd2; d3=dd3; d4=dd4; }

    bool operator == (const ClassID &c) const
      { return (d1==c.d1 && d2==c.d2 && d3==c.d3 && d4==c.d4); }
    
    bool operator == (int zero) const
      { return (d1==0 && d2==0 && d3==0 && d4==0); }

    bool operator != (int zero) const
      { return ! (operator == (0)); }
    
    bool operator < (const ClassID &c) const
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
  ------------------------------------------------------
  Forward declarations
  ------------------------------------------------------*/
  
  typedef IMember* MemberPtr;
  typedef IClass* ClassPtr;
  typedef std::vector <Property*> PTable;
  typedef std::vector <IMember*> MTable;
  typedef std::map <std::string, ClassPtr> CTable;
  typedef std::map <ClassID, ClassPtr> ITable;
  #define INVALID_CLASS_PTR NULL

  /*
  -----------------------------------------------
  Class member descriptor
  -----------------------------------------------*/

  namespace MemberType {
    enum Enum {
      Invalid,
      DataVar,
      DataPtr,
      ObjVar,
      ObjPtr,
      ObjRef,
      ObjArray,
      ObjPtrArray,
      ObjRefArray
    };
  }

  struct MemberInfo
  {
    MemberType::Enum type;
    UintSize size;
    ClassPtr cls;

    MemberInfo () {}
    MemberInfo (MemberType::Enum t, UintSize s, ClassPtr c)
    { type = t; size = s; cls = c; }
  };

  class IMember
  {
  public:
    MemberInfo info;
    std::string name;
    void *data;

    virtual void* getFrom (void* obj) = 0;
    virtual MemberInfo getInfo (void *obj, SerializeManager *sm) = 0;
    virtual bool hasInfoFunc () = 0;
  };

  template <class T, class C> class Member : public IMember
  {
  public:
    typedef MemberInfo (C::*Func) (SerializeManager *sm);
    T C::* ptr;
    Func func;

    void* getFrom (void* obj) {
      return &( ((C*)obj)->*ptr );
    }

    MemberInfo getInfo (void* obj, SerializeManager *sm) {
      return func ? (((C*)obj)->*func)(sm) : info;
    }

    bool hasInfoFunc () {
      return func != NULL;
    }
  };

  /*
  ------------------------------------------------------------
  Info templates. These can be returned by the info function.
  ------------------------------------------------------------*/

  inline MemberInfo MEMBER_SKIP () {
    return MemberInfo( MemberType::Invalid, 0, NULL );
  }

  template <class T, class C>
  inline MemberInfo MEMBER_DATAVAR (T C::* ptr) {
    return MemberInfo( MemberType::DataVar, sizeof(T), NULL );
  }

  template <class T, class C>
  inline MemberInfo MEMBER_OBJVAR (T C::* ptr) {
    return MemberInfo( MemberType::ObjVar, sizeof(T), T::GetClassPtr() );
  }

  template <class T, class C>
  inline MemberInfo MEMBER_OBJPTR (T* C::* ptr) {
    return MemberInfo( MemberType::ObjPtr, sizeof(T), T::GetClassPtr() );
  }

  template <class T, class C>
  inline MemberInfo MEMBER_OBJREF (T* C::* ptr) {
    return MemberInfo( MemberType::ObjRef, sizeof(T), T::GetClassPtr() );
  }

  inline MemberInfo MEMBER_OBJARRAY (ClassPtr cls, UintSize sz) {
    return MemberInfo( MemberType::ObjArray, sz, cls );
  }

  inline MemberInfo MEMBER_OBJPTRARRAY (ClassPtr cls, UintSize sz) {
    return MemberInfo( MemberType::ObjPtrArray, sz, cls );
  }

  inline MemberInfo MEMBER_OBJREFARRAY (ClassPtr cls, UintSize sz) {
    return MemberInfo( MemberType::ObjRefArray, sz, cls );
  }

  inline MemberInfo MEMBER_DATAPTR (UintSize sz) {
    return MemberInfo( MemberType::DataPtr, sz, NULL );
  }
  
  /*
  --------------------------------------------------------------
  Base class descriptor interface
  --------------------------------------------------------------*/
  
  namespace ClassEvent
  {
    enum Enum
    {
      Loaded         = 0,
      Deserialized   = 1,
      Num            = 2
    };
  }
  
  class IClass
  {
  private:

    static CTable* classes;
    static ITable* classesByID;
    
  protected:

    ClassID id;
    std::string name;
    MTable members;
    PTable properties;
    static void Classify (ClassPtr cls);
    
  public:
    
    IClass ();

    //Base layer
    const ClassID&  getID ();
    const char *    getString ();
    const MTable *  getMembers ();
    void            getAllMembers (std::deque<IMember*> &members);
    PTable *        getProperties ();
    
    //Layer-2 (IClass2)
    virtual UintSize  getSize() = 0;
    virtual ClassPtr  getSuper() = 0;
    virtual void      invokeCallback (ClassEvent::Enum e, void *obj, void *param) = 0;
    
    //Layer-3 (IAbstract, IReal, ISerial)
    virtual void* newInstance (int count=1) = 0;
    virtual void* newInPlace (void *pwhere) = 0;
    virtual void* newSerialInstance (SerializeManager *sm) = 0;
    virtual void* newSerialInPlace (void *pwhere, SerializeManager *sm) = 0;
    virtual void  destruct (void *pwhere) = 0;
    virtual void  copy (void *pwhere, void *pfrom) = 0;
    
    //Utilities
    static ClassPtr FromString (const char *name);
    static ClassPtr FromID (ClassID id);
    static void* Safecast (ClassPtr to, ObjectOld *instance);
    /*
    static void SaveText (const ObjectPtr &ptr, ByteString &buf);
    static void SaveBinary (const ObjectPtr &ptr, ByteString &buf);
    static int LoadText (const ObjectPtr &ptr, const ByteString &buf, int index);
    static int LoadBinary (const ObjectPtr &ptr, const ByteString &buf, int index);*/
  };

  /*
  ---------------------------------------------------------------
  A templated class descriptor layer that implements functions
  requiring the class to be known, such as getting the size
  or the parent class as well as performs the delegation role.
  ---------------------------------------------------------------*/

  template <class Name, class Super > class IClass2 : public IClass
  {
  public://Miscellaneous

    UintSize getSize()
      { return sizeof (Name); }
    
    ClassPtr getSuper()
      { return Super::GetClassPtr(); }

  public://Callback datatypes
    
    typedef void (Name::*ClassEventFunc) (void *param);
    
  private:

    ClassEventFunc callbacks[ ClassEvent::Num ];
    
  public://Callback management

    IClass2()
    {
      //Init callback pointers
      for (int i=0; i<ClassEvent::Num; ++i)
        callbacks[ i ] = NULL;
    }

    void registerCallback (ClassEvent::Enum e, ClassEventFunc func)
    {
      //Store callback pointer
      callbacks[ e ] = func;
    }
    
    void invokeCallback (ClassEvent::Enum e, void *obj, void *param)
    {
      Name *nobj = (Name*) obj;

      //Search for callback in this class
      if (callbacks[ e ] != NULL)
        (nobj->*callbacks[ e ]) (param);

      //If not found search in superclass
      else if (getSuper() != this)
        getSuper()->invokeCallback (e, obj, param);
    }

    //Generic templates for adding members of any type    
    template <class T> void addMember (
      const MemberInfo &   i,
      T Name::*            p,
      const std::string &  n,
      void *               d = NULL,
      typename Member<T,Name>::Func f = NULL)
    {
      Member<T,Name> *mbr = new Member<T,Name>;
      mbr->info = i;
      mbr->ptr  = p;
      mbr->name = n;
      mbr->data = d;
      mbr->func = f;
      members.push_back( mbr );
    }
  };
  
  /*
  --------------------------------------------------------------------
  A templated ClassDesc layer that implements instantiation functions
  according to whether the class can be instantiated at all and
  whether it can be serialized or not.
  --------------------------------------------------------------------*/
  
  template <class Name, class Super > class IAbstract
    : public IClass2 <Name, Super > { public:
  
    void* newInstance (int count=1)
      { return NULL; }

    void* newInPlace (void *pwhere)
      { return NULL; }

    void* newSerialInstance (SerializeManager *sm)
      { return NULL; }

    void* newSerialInPlace (void *pwhere, SerializeManager *sm)
      { return NULL; }

    void  destruct (void *pwhere)
      {}

    void  copy (void *pwhere, void *pfrom)
      {}
  };

  template <class Name, class Super > class IReal
    : public IClass2 <Name, Super> { public:
    
    void* newInstance (int count=1)
      { if (count == 1) return (void*) new Name;
        else return (void*) new Name [count]; }
    
    void* newInPlace (void *pwhere)
      { return (void*) new (pwhere) Name; }

    void* newSerialInstance (SerializeManager *sm)
      { return NULL; }
    
    void* newSerialInPlace (void *pwhere, SerializeManager *sm)
      { return NULL; }

    void  destruct (void *pwhere)
      { ((Name*)pwhere) -> ~Name(); }

    void  copy (void *pwhere, void *pfrom)
      { *((Name*)pwhere) = *((Name*)pfrom); }
  };
  
  template <class Name, class Super > class ISerial
    : public IClass2 <Name, Super > { public:
    
    void* newInstance (int count=1)
      { if (count == 1) return (void*) new Name;
        return (void*) new Name [count]; }
    
    void* newInPlace (void *pwhere)
      { return (void*) new (pwhere) Name; }
    
    void* newSerialInstance (SerializeManager *sm)
      { return (void*) new Name (sm); }
    
    void* newSerialInPlace (void *pwhere, SerializeManager *sm)
      { return (void*) new (pwhere) Name (sm); }

    void  destruct (void *pwhere)
      { ((Name*)pwhere) -> ~Name(); }

    void  copy (void *pwhere, void *pfrom)
      { *((Name*)pwhere) = *((Name*)pfrom); }
  };
  
}//namespace GE
#endif //__GECLASS_H

/*
----------------------------------------------------------------
The final layer of the class descriptor implementation, which
is to be tailored appropriately by the user, via the defined
macros.

"Registering" a class means adding a nested descriptor class to
it and static and non-static functions to obtain a unique
instantiation of it. A descriptor is statically allocated for
each class so it doesn't take up memory in each instance. The
uniqueness of the descriptor instance allows checking for class
type to be implemented by comparing the class factory pointer.
----------------------------------------------------------------*/

#define __DECLARE( Interface, Name, Super ) \
public: \
friend class Interface <Name, Super >; \
\
class CLASS_DLL_ACTION ClassDesc : public Interface <Name, Super > { public: \
  \
  typedef Name ThisClass; \
  typedef Super SuperClass; \
  \
  ClassDesc (const char *cname, ClassID cid) { \
    this->name = cname; \
    this->id = cid;
    
    #define DECLARE_CALLBACK( evnt, func ) \
    registerCallback (evnt, &ThisClass::func);

    #define DECLARE_DATAVAR( mbr ) \
    addMember( MEMBER_DATAVAR( &ThisClass::mbr ), &ThisClass::mbr, #mbr);

    #define DECLARE_OBJVAR( mbr )\
    addMember( MEMBER_OBJVAR( &ThisClass::mbr ), &ThisClass::mbr, #mbr );

    #define DECLARE_OBJPTR( mbr )\
    addMember( MEMBER_OBJPTR( &ThisClass::mbr ), &ThisClass::mbr, #mbr );

    #define DECLARE_OBJREF( mbr )\
    addMember( MEMBER_OBJREF( &ThisClass::mbr ), &ThisClass::mbr, #mbr );

    #define DECLARE_MEMBER_DATA( mbr, data ) \
    addMember( MEMBER_DATAVAR( &ThisClass::mbr ), &ThisClass::mbr, #mbr, data );

    #define DECLARE_MEMBER_FUNC( mbr, func ) \
    addMember( MEMBER_DATAVAR( &ThisClass::mbr ), &ThisClass::mbr, #mbr, NULL, &ThisClass::func );
    
    #define DECLARE_END \
    IClass::Classify (this); \
  } \
}; \
\
friend class ClassDesc; \
static ClassDesc classDesc; \
\
static ClassPtr GetClassPtr() { \
  return &classDesc; \
} \
virtual ClassPtr GetInstanceClassPtr() { \
  return &classDesc; \
} \
private:


#define DECLARE_CLASS( Name ) __DECLARE( IReal, Name, Name )
#define DECLARE_SUBCLASS( Name, Super ) __DECLARE( IReal, Name, Super )

#define DECLARE_ABSTRACT( Name ) __DECLARE( IAbstract, Name, Name )
#define DECLARE_SUBABSTRACT( Name, Super ) __DECLARE( IAbstract, Name, Super )

#define DECLARE_SERIAL_CLASS( Name ) __DECLARE( ISerial, Name, Name )
#define DECLARE_SERIAL_SUBCLASS( Name, Super ) __DECLARE( ISerial, Name, Super )

#define DEFINE_CLASS( Name ) Name::ClassDesc Name::classDesc (#Name, ClassID())
#define DEFINE_SERIAL_CLASS( Name, ID ) Name::ClassDesc Name::classDesc (#Name, ID)
#define DEFINE_TEMPL_CLASS( Name ) template <> Name::ClassDesc Name::classDesc (#Name, ClassID())
#define DEFINE_SERIAL_TEMPL_CLASS( Name, ID ) template <> Name::ClassDesc Name::classDesc (#Name, ID)



#ifndef __GECLASS_H_TWO
#define __GECLASS_H_TWO
namespace GE
{
  class ObjectOld
  {
    DECLARE_SERIAL_CLASS( ObjectOld );
    DECLARE_END;

  public:
    UintSize serialID;
    ObjectOld (SerializeManager *sm) : serialID(0) {}
    ObjectOld () : serialID(0) {}
    virtual ~ObjectOld() {}
  };

  /*
  --------------------------------------------------------------
  End-user macros to beautify class management
  --------------------------------------------------------------*/

  //ClassPtr from a given class
  #define Class( Name ) Name::GetClassPtr()

  //ClasPtr from an object instance
  #define ClassOf( instance ) (instance)->GetInstanceClassPtr()

  //ClassPtr from string
  #define ClassFromString( name ) IClass::FromString(name)
  #define ClassFromID( id ) IClass::FromID(id)

  //operations on a stored IClass*
  #define SuperOf( iclass ) (iclass)->getSuper()
  #define StringOf( iclass ) (iclass)->getString()
  #define New( iclass ) iclass->newInstance()

  //safecasting an instance pointer to another type
  #define SafeCastPtr( classPtr, instance ) \
  (void*) IClass::Safecast( classPtr , instance )
  
  #define SafeCast( Name, instance ) \
  (Name*) IClass::Safecast( Class(Name), instance )


  /*
  --------------------------------------------------------
  Known class IDs
  --------------------------------------------------------*/

  #define CLSID_SIGNATURE            ClassID (0xbbfb896eu, 0x3c7e, 0x4a83, 0xb99d110bc4a7c6d0ull)
  #define CLSID_OBJECT               ClassID (0x6dcd04eeu, 0x5e7b, 0x4194, 0xa09a3ce4a2986f3full)
  #define CLSID_CHARSTRING           ClassID (0xd7f2841bu, 0xadbc, 0x4d81, 0xbc5889e771dd4496ull)
  #define CLSID_BYTESTRING           ClassID (0xf7bca47cu, 0x4089, 0x4d7f, 0xb22d9bab68d87c42ull)
  #define CLSID_UNICODESTRING        ClassID (0xf54b6286u, 0xaebb, 0x48fb, 0x84cf8ffaca844ed8ull)
  #define CLSID_GENARRAYLIST         ClassID (0x66fd1aa8u, 0xc068, 0x4072, 0x8699a02e75e1a55dull)
  #define CLSID_GENOBJARRAYLIST      ClassID (0xc26bed4fu, 0x0944, 0x4f44, 0xa7efa5605759ca05ull)
  #define CLSID_GENPTRARRAYLIST      ClassID (0x066fd45au, 0x6338, 0x48f9, 0x87ebf565239dbea9ull)
  #define CLSID_VERTEXFORMAT         ClassID (0x0ad07051u, 0xb8c1, 0x4f5d, 0x8a806c62dd16cf3full)
  #define CLSID_FORMATMEMBER         ClassID (0x95fea7e0u, 0x527d, 0x43fe, 0xb5018eebd7fa36a5ull)
  #define CLSID_TRIMESH              ClassID (0x827c9bdfu, 0x8e80, 0x47bf, 0x8c6b2f0505bcdd6dull)
  #define CLSID_SKINTRIMESH          ClassID (0x71186776u, 0xe0e9, 0x428a, 0xb910329aafd3392full)
  #define CLSID_SKINMESH             ClassID (0xb9a1c7cdu, 0xcf04, 0x46b3, 0x837765467e60293bull)
  #define CLSID_SKINPOSE             ClassID (0x4b2c0c9au, 0xa47f, 0x4675, 0x88e0bf3aa5955a16ull)

}//namespace GE
#endif//__GECLASS_H_TWO
