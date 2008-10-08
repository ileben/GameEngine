#ifndef __GEKERNEL_H
#define __GEKERNEL_H
#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  typedef void (*PFVOID)();

  /*
  -------------------------------------
  Forward declarations
  -------------------------------------*/
  class Kernel;
  
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
  
  class GE_API_ENTRY Kernel
  {
    DECLARE_CLASS (Kernel); DECLARE_END;
    friend class Renderer;
    friend class TriMesh;
    
  private:
    
    static Kernel* Instance;
    
    PFVOID getProcAddress (const char *name);
    int checkExtension (const char *extensions, const char *name);
    
    int textureUnits;
    bool hasMultitexture;
    bool hasShaders;
    bool hasRangeElements;
    int maxElementsIndices;
    int maxElementsVertices;
    void loadExtensions ();
    
    ArrayListT<ObjectPtr> objects;
    OCC::ArraySet<KernelBuffer*> buffers;
    
  public:
    
    Kernel();
    ~Kernel();
    
    KernelBuffer* createBuffer (int size);
    void refBuffer (KernelBuffer* buf);
    void derefBuffer (KernelBuffer *buf);
    
    void* spawn (ClassPtr classPtr);
    void* spawn (const char *classString);
    void enableVerticalSync (bool on);
  };
}

#pragma warning(pop)
#endif /* __GEKERNEL_H */
