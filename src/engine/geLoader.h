#ifndef __GELOADER_H
#define __GELOADER_H

#include "util/geUtil.h"

namespace GE
{
  /*
  ----------------------------------------------------
  Forward declarations
  ----------------------------------------------------*/

  class Actor3D;
  class Resource;

  /*
  ----------------------------------------------------
  Loader class loads objects from multiple file
  formats and builts a temporary scene tree.
  The objects, selected to be "retained" by the
  user are preserved while the rest is automatically
  deleted when the Loader class is destroyed
  ----------------------------------------------------*/

#pragma warning(push)
#pragma warning(disable:4251)

  class GE_API_ENTRY Loader
  {
    DECLARE_CLASS (Loader); DECLARE_END;

  protected:
    //Resources
    ArrayList<Resource*> resources;
    
    //Scene tree
    Actor3D *root;
    ArrayList<Actor3D*> objects;
    void deleteObject( Actor3D *obj );

    //Specific file type loaders. These can call
    //each other in case additional resources are
    //referenced inside a certain file
    void loadTex( const String &filename );
    void load3ds( const String &filename );

    //Classes to use when creating objects
    ClassPtr uvMeshClass;
    ClassPtr dMeshClass;
    ClassPtr sMeshClass;

  public:
    Loader();
    ~Loader();

    //Setters for class names
    void setUVMeshClass( ClassPtr name );
    void setDMeshClass( ClassPtr name );
    void setSMeshClass( ClassPtr name );

    //Loads any file with prober sub-routine
    virtual bool loadFile( const String &filename );
    
    //Getters for loaded stuff
    Actor3D* getRoot();
    Actor3D* getFirstObject(ClassPtr type);
    Resource* getFirstResource(ClassPtr type);
    const ArrayList <Resource*> * getResources();
    
    void retainObject(Actor3D *obj);
  };

#pragma warning(pop)
}

#endif /* __GELOADER_H */
