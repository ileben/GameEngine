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
  ClassDesc (const char *newName) : Interface <Name > (newName) { \
    
    #define DECLARE_END \
  } \
  IClass* getSuper () { \
    return SuperClass::GetClassDesc (); \
  } \
}; \
\
static ClassDesc classDesc; \
\
static IClass* GetClassDesc() { \
  return &classDesc; \
} \
virtual IClass* GetInstanceClassDesc() { \
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
  
  typedef OCC::ArrayList <Property*> PTable;
  typedef std::map<std::string, IClass*> CTable;
  
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
    static IClass* FromString (const char *name);
    
  protected:
    OCC::CharString name;
    PTable properties;
    
  public:
    IClass (const char *newName);
    const char* getString ();
    PTable *getProperties ();
    virtual void* getInstance() = 0;
    virtual IClass* getSuper() = 0;
    
    static void SaveText (IClass *desc, void *obj, OCC::ByteString &buf);
    static void SaveBinary (IClass *desc, void *obj, OCC::ByteString &buf);
    static int LoadText (IClass *desc, void *obj, const OCC::ByteString &buf, int index);
    static int LoadBinary (IClass *desc, void *obj, const OCC::ByteString &buf, int index);
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
  
  typedef IClass* ClassName;
  
  //IClass* from a given class
  #define Class( Name ) Name::GetClassDesc()
  
  //IClass* from an object instance
  #define ClassOf( instance ) (instance)->GetInstanceClassDesc()
  
  //IClass* from string
  #define ClassFromString( name ) IClass::FromString(name)
  
  //operations on a stored IClass*
  #define SuperOf( iclass ) (iclass)->getSuper()
  #define StringOf( iclass ) (iclass)->getString()
  #define New( iclass ) iclass->getInstance()
  
  //safecasting an instance pointer to another type
  GE_API_ENTRY void* Safecast (ClassName to, ClassName from, void *instance);
  #define SafeCastName( name, instance ) \
  (void*) Safecast (name, ClassOf(instance), instance)
  #define SafeCast( Name, instance ) \
  (Name*) Safecast (Class(Name), ClassOf(instance), instance)


}//namespace GE
#endif //__GECLASS_H
