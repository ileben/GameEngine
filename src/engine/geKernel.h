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
    bool hasShaderObjects;
    bool hasFramebufferObjects;
    bool hasVertexBufferObjects;
    bool hasRangeElements;
    int maxElementsIndices;
    int maxElementsVertices;
    void loadExtensions ();
    
    ArrayList<ObjectPtr> objects;
    ArraySet<KernelBuffer*> buffers;
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

    static Kernel* GetInstance ();
    Renderer* getRenderer ();
  };
}

#pragma warning(pop)
#endif /* __GEKERNEL_H */
