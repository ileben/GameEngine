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

  class GE_API_ENTRY Shape : public Actor
  {
    DECLARE_SUBCLASS (Shape, Actor); DECLARE_END;
    friend class Renderer;
    friend class SaverObj;

  protected:
    UMesh *uvMesh;
    SMesh *statMesh;
    PolyMesh *dynMesh;
    bool useDynamic;
    
    virtual void renderDynamic (MaterialId materialId);
    virtual void renderStatic (MaterialId materialId);
    
  public:
    Shape();
    ~Shape();
    void setUV(UMesh *mesh);
    void setStatic(SMesh *mesh);
    void setDynamic(PolyMesh *mesh);
    UMesh* getUV();
    SMesh* getStatic();
    PolyMesh* getDynamic();
    
    virtual void render (MaterialId materialId);
  };
}

#endif /* __GESHAPE_H */
