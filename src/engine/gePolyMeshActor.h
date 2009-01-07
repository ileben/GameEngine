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
    TexMesh *texMesh;
    PolyMesh *polyMesh;
    
    virtual void renderMesh (MaterialID materialID);
    
  public:
    PolyMeshActor();
    ~PolyMeshActor();

    void setMesh (PolyMesh *mesh);
    void setTexMesh (TexMesh *mesh);

    PolyMesh* getMesh();
    TexMesh* getTexMesh ();
    
    virtual void render (MaterialID materialID);
  };
}

#endif /* __GESHAPE_H */
