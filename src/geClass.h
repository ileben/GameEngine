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
  ClassDesc (const char *cname) : Interface <Name > (cname) { \
    
    #define DECLARE_CREATOR( func ) \
    creator = new Creator <ThisClass> (&ThisClass::func);
    
    #define DECLARE_PROPERTY( Type, pname ) \
    properties.pushBack (new TypeProperty <Type, ThisClass> (&ThisClass::pname, #pname));
    
    #define DECLARE_END \
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

#define DEFINE_CLASS( Name ) Name::ClassDesc Name::classDesc (#Name)

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
  
  typedef IClass* ClassPtr;
  typedef OCC::ArrayList <Property*> PTable;
  typedef std::map<std::string, ClassPtr> CTable;
  
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
    
  public:
    static ClassPtr FromString (const char *name);
    
  protected:
    OCC::CharString name;
    PTable properties;
    ICreator *creator;
    
  public:
    IClass (const char *newName);
    const char* getString ();
    PTable *getProperties ();
    virtual ClassPtr getSuper() = 0;
    virtual void* getInstance() = 0;
    
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
  
  template <class Name > class IReal : public IClass { public:
    IReal (const char *newName) : IClass (newName) {}
    void* getInstance() { return (void*) new Name; }
  };
  
  template <class Name > class IAbstract : public IClass { public:
    IAbstract (const char *newName) : IClass (newName) {}
    void* getInstance() { return NULL; }
  };
  
  /*
  --------------------------------------------------------------
  End-user macros to beautify class management
  --------------------------------------------------------------*/
  
  //IClass* from a given class
  #define Class( Name ) Name::GetClassPtr()
  
  //IClass* from an object instance
  #define ClassOf( instance ) (instance)->GetInstanceClassPtr()
  
  //IClass* from string
  #define ClassFromString( name ) IClass::FromString(name)
  
  //operations on a stored IClass*
  #define SuperOf( iclass ) (iclass)->getSuper()
  #define StringOf( iclass ) (iclass)->getString()
  #define New( iclass ) iclass->getInstance()
  
  //safecasting an instance pointer to another type
  GE_API_ENTRY void* Safecast (ClassPtr to, ClassPtr from, void *instance);
  #define SafeCastName( name, instance ) \
  (void*) Safecast (name, ClassOf(instance), instance)
  #define SafeCast( Name, instance ) \
  (Name*) Safecast (Class(Name), ClassOf(instance), instance)


}//namespace GE
#endif //__GECLASS_H
