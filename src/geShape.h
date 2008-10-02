#ifndef __GESHAPE_H
#define __GESHAPE_H

namespace GE
{
  /*----------------------------------------------
   * Forward declarations
   *----------------------------------------------*/
  class Renderer;
  class SaverObj;

  /*----------------------------------------------
   * A renderable object. Its geometry is defined
   * with either a static or animated mesh. The
   * material defines how geometry is finally
   * rendered.
   *----------------------------------------------*/

  class GE_API_ENTRY PolyMeshActor : public Actor
  {
    friend class Renderer;
    friend class SaverObj;
    DECLARE_SUBCLASS (PolyMeshActor, Actor);
    DECLARE_END;

  protected:
    UMesh *texMesh;
    PolyMesh *polyMesh;
    
    virtual void renderMesh (MaterialId materialId);
    
  public:
    PolyMeshActor();
    ~PolyMeshActor();

    void setMesh (PolyMesh *mesh);
    void setTexMesh (UMesh *mesh);

    PolyMesh* getMesh();
    UMesh* getTexMesh ();
    
    virtual void render (MaterialId materialId);
  };
}

#endif /* __GESHAPE_H */
