#ifndef __GELOADER_H
#define __GELOADER_H

namespace GE
{
  /*----------------------------------------------------
   * Loader class loads objects from multiple file
   * formats and builts a temporary scene tree.
   * The objects, selected to be "retained" by the
   * user are preserved while the rest is automatically
   * deleted when the Loader class is destroyed
   *----------------------------------------------------*/

#pragma warning(push)
#pragma warning(disable:4251)

  class GE_API_ENTRY Loader
  {
    DECLARE_CLASS (Loader); DECLARE_END;

  protected:
    //Resources
    OCC::ArrayList<Resource*> resources;
    
    //Scene tree
    Group *root;
    OCC::ArrayList<Actor*> objects;
    void deleteObject(Actor *obj);

    //Specific file type loaders. These can call
    //each other in case additional resources are
    //referenced inside a certain file
    void loadTex(const OCC::String &filename);
    void load3ds(const OCC::String &filename);

    //Classes to use when creating objects
    ClassName uvMeshClass;
    ClassName dMeshClass;
    ClassName sMeshClass;

  public:
    Loader();
    ~Loader();

    //Setters for class names
    void setUVMeshClass(ClassName name);
    void setDMeshClass(ClassName name);
    void setSMeshClass(ClassName name);

    //Loads any file with prober sub-routine
    virtual bool loadFile(const OCC::String &filename);
    
    //Getters for loaded stuff
    Group* getRoot();
    Actor* getFirstObject(ClassName type);
    Resource* getFirstResource(ClassName type);
    const OCC::ArrayList<Resource*>* getResources();
    
    void retainObject(Actor *obj);
  };

#pragma warning(pop)
}

#endif /* __GELOADER_H */
