#include "geLoader.h"
#include "geActor.h"
#include "geTexMesh.h"
#include "gePolyMesh.h"
#include "geTriMesh.h"

namespace GE
{
  DEFINE_CLASS (Loader);
  
  Loader::Loader()
  {
    root = NULL;
    uvMeshClass = Class( TexMesh );
    dMeshClass = Class( PolyMesh );
    sMeshClass = Class( TriMesh );
  }

  Loader::~Loader()
  {
    //delete object tree
     if( root != NULL )
       deleteObject( root );

    //delete unreferenced resources
    for( UintSize r=0; r<resources.size(); ++r )
      if( resources[r]->getRefCount() == 0 )
        delete resources[r];
  }

  /*------------------------------------
   * Setters for class names
   *------------------------------------*/

  void Loader::setUVMeshClass(ClassPtr name) {
    //TODO: check whether given class is really derived from UVMesh
    uvMeshClass = name;
  }

  void Loader::setDMeshClass(ClassPtr name) {
    dMeshClass = name;
  }

  void Loader::setSMeshClass(ClassPtr name) {
    sMeshClass = name;
  }

  /*-------------------------------------
   * Recursively deletes the scene tree
   *------------------------------------*/

  void Loader::deleteObject(Actor3D *obj)
  {
    //delete children if any
    const ArrayList<Actor3D*> *children = obj->getChildren();
    for( UintSize c=0; c<children->size(); ++c )
      deleteObject( children->at(c) );
    
    delete obj;
  }

  /*-------------------------------------
   * Returns the first loaded resource
   * of given type or NULL if none found
   *-------------------------------------*/

  Resource* Loader::getFirstResource (ClassPtr type)
  {
    //Search for first resource of given type
    for( UintSize r=0; r<resources.size(); ++r )
      if( SafeCastName( type, resources[r] ))
        return resources[r];
    
    return NULL;
  }

  /*-------------------------------------
   * Returns the first loaded object
   * of given type or NULL if none found
   *-------------------------------------*/

  Actor3D* Loader::getFirstObject (ClassPtr type)
  {
    //Search for first object of given type
    for( UintSize o=0; o<objects.size(); ++o )
      if( SafeCastName( type, objects[o] ))
        return objects[o];
    
    return NULL;
  }
  
  /*-------------------------------------
   * Returns a pointer to unmodifiable
   * list of all the loaded resources
   *-------------------------------------*/

  const ArrayList<Resource*>* Loader::getResources()
  {
    return &resources;
  }

  /*--------------------------------------------
   * Marks an object to be retained by user and
   * not destroyed along with Loader class
   *--------------------------------------------*/

  void Loader::retainObject( Actor3D *obj )
  {
    //TODO: hmmmmm.......
  }

  /*--------------------------------------------
   * Opens the file and loads all the resources
   * or the whole scene tree found inside
   *--------------------------------------------*/

  bool Loader::loadFile( const String &filename )
  {
    /*
    String ending = filename.right(4);

    if (ending.compare(".3ds", false)) {

      load3ds(filename);

    }else if (ending.compare(".jpg"), false) ||
              ending.compare(".jpeg"), false) ||
              ending.compare(".png"), false)) {

      loadTex(filename);
    }*/
    return true;
  }
}
