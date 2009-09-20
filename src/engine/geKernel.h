#ifndef __GEKERNEL_H
#define __GEKERNEL_H

#include "util/geUtil.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  typedef void (*PFVOID)();

  /*
  -------------------------------------
  Forward declarations
  -------------------------------------*/
  class Renderer;
  class Kernel;
  class Resource;
  class Texture;
  class TriMesh;
  class Character;
  class ResourceRef;
  class Scene3D;
  
  /*
  -------------------------------------
  Kernel-managed memory
  -------------------------------------*/

  class GE_API_ENTRY KernelBuffer
  {
    friend class Kernel;

  private:
    int refcount;
    int size;
    void *data;
    
    KernelBuffer (int newSize);
    ~KernelBuffer ();
    void reference ();
    void dereference ();
  };
  
  /*
  --------------------------------------------
  Kernel interface (singleton!)
  --------------------------------------------*/
  
  typedef std::map <std::string, Resource*> ResourceMap;
  typedef ResourceMap::iterator ResourceIter;

  class Kernel : public Object
  {
    DECLARE_SUBCLASS( Kernel, Object );
    DECLARE_END;

    friend class Renderer;
    friend class TriMesh;
    
  private:
    
    static Kernel* Instance;
    
    PFVOID getProcAddress (const char *name);
    int checkExtension (const char *extensions, const char *name);
    
    int textureUnits;
    bool hasMultitexture;
    bool hasShaderObjects;
    bool hasFramebufferObjects;
    bool hasVertexBufferObjects;
    bool hasMultipleRenderTargets;
    bool hasDepthStencilFormat;
    bool hasRangeElements;
    bool hasOcclusionQuery;
    int maxOcclusionBits;
    int maxRenderTargets;
    int maxElementsIndices;
    int maxElementsVertices;
    void loadExtensions ();
    
    ArrayList<Object*> objects;
    ArraySet<KernelBuffer*> buffers;

    std::map< std::string, Resource* > resources;
    Renderer *renderer;

    bool timeInit;
    Float time;
    Float dtime;
    
  public:
    
    Kernel();
    ~Kernel();
    
    KernelBuffer* createBuffer (int size);
    void refBuffer (KernelBuffer* buf);
    void derefBuffer (KernelBuffer *buf);
    
    void* spawn (ClassPtr classPtr);
    void* spawn (const char *classString);
    void enableVerticalSync (bool on);
    void tick (Float time);
    Float getTime ();
    Float getInterval ();

    static Kernel* GetInstance ()
    { return Kernel::Instance; }

    Renderer* getRenderer ();

    void cacheResource (Resource *res, const CharString &name);
    Resource* getResource (const CharString &name);
    Scene3D* loadSceneFile (const CharString &filename);
    Scene3D* loadSceneData (void *data);
  };

  /*
  ---------------------------------------
  Reference to resources
  ---------------------------------------*/

  class ResourceRef : public Object
  {
    DECLARE_SERIAL_SUBCLASS( ResourceRef, Object );
    DECLARE_OBJVAR( name );
    DECLARE_END;

  public:

    void *ptr;
    CharString name;

    ResourceRef (SM *sm) : name(sm)
    { ptr = NULL; }

    ResourceRef ()
    { ptr = NULL; }
  };

  template <class T> class TResourceRef : public ResourceRef
  {
  public:
    TResourceRef () {}
    TResourceRef (SM *sm) : ResourceRef (sm) {}
    T* operator-> () { return (T*) ptr; }
    T& operator* () { return *( operator->() ); }
    void operator= (T *t) { ptr = t; name = (t != NULL) ? t->getResourceName() : ""; }
    void operator= (const CharString &n) { ptr = NULL; name = n; }
    bool operator== (T *t) { return (T*)ptr == t; }
    operator T* () { return (T*) ptr; }
  };

  typedef TResourceRef<Texture> TextureRef;
  typedef TResourceRef<TriMesh> MeshRef;
  typedef TResourceRef<Character> CharRef;
}

#pragma warning(pop)
#endif /* __GEKERNEL_H */
