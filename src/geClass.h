#pragma warning(push)
#pragma warning(disable:4251)

/*
----------------------------------------------------------------
DECLARE_DLL_SPEC should be #defined to the keyword specifiying
dll export or import action for the current declaration!
----------------------------------------------------------------*/

#ifndef CLASS_DLL_ACTION
#  define CLASS_DLL_ACTION
#endif

/*
----------------------------------------------------------------
"Registering" a class means adding a nested descriptor class to
it and static and non-static functions to obtain a unique
instantiation of it. A descriptor is statically allocated for
each class so it doesn't take up memory in each instance. The
uniqueness of the descriptor instance allows checking for class
type to be implemented by comparing the class factory pointer.
----------------------------------------------------------------*/

#define __DECLARE( Interface, Name, Super ) \
public: \
\
class CLASS_DLL_ACTION ClassDesc : public Interface <Name > { public: \
  \
  typedef Name ThisClass; \
  typedef Super SuperClass; \
  \
  ClassDesc (const char *cname, ClassID cid) { \
    name = cname; \
    id = cid; \
    size = sizeof (Name);
    
    #define DECLARE_CREATOR( func ) \
    creator = new Creator <ThisClass> (&ThisClass::func);
    
    #define DECLARE_PROPERTY( Type, pname ) \
    properties.pushBack (new TypeProperty <Type, ThisClass> (&ThisClass::pname, #pname));
    
    #define DECLARE_END \
    IClass::Classify (this); \
  } \
  ClassPtr getSuper () { \
    return SuperClass::GetClassPtr(); \
  } \
}; \
\
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
#define DECLARE_SERIAL_SUBCLASS( Name ) __DECLARE( ISerial, Name, Name )

#define DEFINE_CLASS( Name ) Name::ClassDesc Name::classDesc (#Name, ClassID())
#define DEFINE_SERIAL_CLASS( Name, ID ) Name::ClassDesc Name::classDesc (#Name, ID);

/*
-------------------------------------------------------------
The rest can only be #included once.
-------------------------------------------------------------*/

#ifndef __GECLASS_H
#define __GECLASS_H

#include <map>

namespace GE
{

  class IClass;
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
    
    ClassID ()
      { d1=0; d2=0; d3=0; d4=0; }

    ClassID (Uint32 dd1, Uint16 dd2, Uint16 dd3, Uint64 dd4)
     { d1=dd1; d2=dd2; d3=dd3; d4=dd4; }
    
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
  inline bool operator == (const ClassID &c, int zero)
    { return (c.d1==0 && c.d2==0 && c.d3==0 && c.d4==0); }
  
  inline bool operator != (const ClassID &c, int zero)
    { return ! (operator == (c, 0)); }
  
  inline bool operator < (const ClassID &c1, const ClassID &c2)
  {
    if (c1.d1 < c2.d1) return true;
    if (c1.d1 > c2.d1) return false;
    
    if (c1.d2 < c2.d2) return true;
    if (c1.d2 > c2.d2) return false;
    
    if (c1.d3 < c2.d3) return true;
    if (c1.d3 > c2.d3) return false;
    
    if (c1.d4 < c2.d4) return true;
    return false;
  } */
  
  typedef IClass* ClassPtr;
  typedef OCC::ArrayList <Property*> PTable;
  typedef std::map <std::string, ClassPtr> CTable;
  typedef std::map <ClassID, ClassPtr> ITable;
  
  /*
  --------------------------------------------------------
  A generic reference to an object, holding a pointer to
  its class descriptor and a pointer to an instance.
  This is a workaround for not using any specific base
  such as "Object". It allows for third-party classes or
  their custom subclasses to have the class interface
  attached to them.
  --------------------------------------------------------*/
  
  class ObjectPtr
  {
  public:
    ClassPtr cls;
    void *obj;
    
    ObjectPtr () {}
    ObjectPtr (ClassPtr c, void *o) {cls=c; o=o;}
    ObjectPtr (const ObjectPtr &p) {cls=p.cls; obj=p.obj;}
    template <class T> ObjectPtr (T *o) {cls=o->GetInstanceClassPtr(); obj=o;}
  };
  
  /*
  ----------------------------------------------------------
  A simple helper class to store member function pointer to
  the create callback of the class in the template argument
  ----------------------------------------------------------*/
  
  class ICreator { public:
    virtual void create (void *obj, void *data, int size) = 0;
  };
  
  template <class C> class Creator : public ICreator { public:
    
    typedef void (C::*CreateFunc) (void *data, int size);
    CreateFunc createCallback;
    
    Creator (CreateFunc f)
      { createCallback = f; }
    
    void create (void *obj, void *data, int size)
      { (((C*)obj)->*createCallback) (data, size); }
  };
  
  /*
  --------------------------------------------------------------
  Base class descriptor interface to be able to use the generic
  pointer to point to any class descriptor
  --------------------------------------------------------------*/
  
  class GE_API_ENTRY IClass
  {
  private:
    static CTable* classes;
    static ITable* classesByID;
  public:
    static ClassPtr FromString (const char *name);
    static ClassPtr FromID (ClassID id);
    
  protected:
    ClassID id;
    UintP size;
    OCC::CharString name;
    PTable properties;
    ICreator *creator;
    
    static void Classify (ClassPtr cls);
    
  public:
    IClass ();
    const ClassID& getID ();
    const char* getString ();
    UintP getSize();
    PTable *getProperties ();
    virtual ClassPtr getSuper() = 0;
    
    virtual void* newInstance () = 0;
    virtual void* newInPlace (void *pwhere) = 0;
    virtual void* newDeserialized (void *pmem, SerializeManager *sm) = 0;
    
    static void SaveText (const ObjectPtr &ptr, OCC::ByteString &buf);
    static void SaveBinary (const ObjectPtr &ptr, OCC::ByteString &buf);
    static int LoadText (const ObjectPtr &ptr, const OCC::ByteString &buf, int index);
    static int LoadBinary (const ObjectPtr &ptr, const OCC::ByteString &buf, int index);
    static void Create (const ObjectPtr &ptr, void *buf, int size);
  };
  
  /*
  -------------------------------------------------------------------
  Templated glue-class to be able to allow instantiation of concrete
  class types but avoid compile errors for abstract classes.
  -------------------------------------------------------------------*/
  
  template <class Name > class IAbstract : public IClass { public:
    void* newInstance () { return NULL; }
    void* newInPlace (void *pmem) { return NULL; }
    void* newDeserialized (void *pmem, SerializeManager *sm) { return NULL; }
  };

  template <class Name > class IReal : public IClass { public:
    
    void* newInstance ()
      { return (void*) new Name; }
    
    void* newInPlace (void *pwhere)
      { return (void*) new (pwhere) Name; }
    
    void* newDeserialized (void *pwhere, SerializeManager *sm)
      { return NULL; }
  };
  
  template <class Name > class ISerial : public IClass { public:

    void* newInstance ()
      { return (void*) new Name; }
    
    void* newInPlace (void *pwhere)
      { return (void*) new (pwhere) Name; }
    
    void* newDeserialized (void *pwhere, SerializeManager *sm)
      { return (void*) new (pwhere) Name (sm); }
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
  GE_API_ENTRY void* Safecast (ClassPtr to, ClassPtr from, void *instance);
  #define SafeCastName( name, instance ) \
  (void*) Safecast (name, ClassOf(instance), instance)
  #define SafeCast( Name, instance ) \
  (Name*) Safecast (Class(Name), ClassOf(instance), instance)

  
  /*
  --------------------------------------------------------
  Known class IDs
  --------------------------------------------------------*/

  #define CLSID_ARRAYLIST_RES_I32    ClassID (0x66fd1aa8, 0xc068, 0x4072, 0x8699a02e75e1a55d)
  #define CLSID_ARRAYLIST_RES_SPMV   ClassID (0x515566aa, 0x2d91, 0x484f, 0xbff8163332271bbc)
  #define CLSID_ARRAYLIST_RES_SPMF   ClassID (0xeb0aeac2, 0xa1d0, 0x42cc, 0x883687e9ca802c0f)
  #define CLSID_SKINPOLYMESH_RES     ClassID (0xb9a1c7cd, 0xcf04, 0x46b3, 0x837765467e60293b)

  #define CLSID_ARRAYLIST_RES_SB     ClassID (0xd33ab9dd, 0x4431, 0x4c2b, 0xaf271711f5e30ad5)
  #define CLSID_SKELETON_RES         ClassID (0x4b2c0c9a, 0xa47f, 0x4675, 0x88e0bf3aa5955a16)

  #define CLSID_MAXCHARACTER_RES  ClassID (0xc0db7169, 0x65dd, 0x4375, 0xa4b2d9a505703db8)


}//namespace GE
#endif //__GECLASS_H
