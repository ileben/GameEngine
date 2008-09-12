#ifndef __GEKERNEL_H
#define __GEKERNEL_H
#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  typedef void (*PFVOID)();
  
  class GE_API_ENTRY Kernel
  {
    DECLARE_CLASS (Kernel); DECLARE_END;
    friend class SMesh;
    friend class Renderer;
    
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
    
    OCC::ArrayList<Object*> objects;
    
  public:
    
    Kernel();
    ~Kernel();
    
    Object* spawn (ClassName className);
    Object* spawn (const char *classString);
    void enableVerticalSync (bool on);
  };
}

#pragma warning(pop)
#endif /* __GEKERNEL_H */
