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

  private:
    UMesh *uvMesh;
    SMesh *statMesh;
    DMesh *dynMesh;
    Material *material;
    bool useDynamic;
    
  public:
    Shape();
    ~Shape();
    void setUV(UMesh *mesh);
    void setStatic(SMesh *mesh);
    void setDynamic(DMesh *mesh);
    void setMaterial(Material *material);
    UMesh* getUV();
    SMesh* getStatic();
    DMesh* getDynamic();
    Material* getMaterial();
  };
}

#endif /* __GESHAPE_H */
