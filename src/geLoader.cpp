#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Loader);
  
  Loader::Loader()
  {
    root = NULL;
    uvMeshClass = Class(UMesh);
    dMeshClass = Class(PolyMesh);
    sMeshClass = Class(TriMesh);
  }

  Loader::~Loader()
  {
    //delete object tree
     if (root != NULL)
       deleteObject(root);

    //delete unreferenced resources
    for (ArrayList<Resource*>::Iterator it=resources.begin(); it!=resources.end(); ++it)
      if ((*it)->getRefCount() == 0) delete *it;
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

  void Loader::deleteObject(Actor *obj)
  {
    //delete children if group
    if (ClassOf(obj) == Class(Group)) {
      const OCC::ArrayList<Actor*> *children = ((Group*)obj)->getChildren();
      for (ArrayList<Actor*>::Iterator it=children->begin();
           it!=children->end(); ++it) deleteObject(*it);
    }

    delete obj;
  }

  /*-------------------------------------
   * Returns the first loaded resource
   * of given type or NULL if none found
   *-------------------------------------*/

  Resource* Loader::getFirstResource (ClassPtr type)
  {
    //Search for first resource of given type
    for (ArrayList<Resource*>::Iterator it=resources.begin(); it!=resources.end(); ++it)
      if (SafeCastName (type, *it))
        return *it;
    
    return NULL;
  }

  /*-------------------------------------
   * Returns the first loaded object
   * of given type or NULL if none found
   *-------------------------------------*/

  Actor* Loader::getFirstObject (ClassPtr type)
  {
    //Search for first object of given type
    for (ArrayList<Actor*>::Iterator it=objects.begin(); it!=objects.end(); ++it)
      if (SafeCastName (type, *it))
        return *it;
    
    return NULL;
  }

  /*-------------------------------------
   * Returns a pointer to unmodifiable
   * list of all the loaded resources
   *-------------------------------------*/

  const OCC::ArrayList<Resource*>* Loader::getResources()
  {
    return &resources;
  }

  /*--------------------------------------------
   * Marks an object to be retained by user and
   * not destroyed along with Loader class
   *--------------------------------------------*/

  void Loader::retainObject(Actor *obj)
  {
    //TODO: hmmmmm.......
  }

  /*--------------------------------------------
   * Opens the file and loads all the resources
   * or the whole scene tree found inside
   *--------------------------------------------*/

  bool Loader::loadFile(const String &filename)
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
