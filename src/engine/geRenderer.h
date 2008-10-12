#ifndef __GERENDERER_H
#define __GERENDERER_H

namespace GE
{
  /*----------------------------------------
   * Renders a scene or single object
   *----------------------------------------*/

  class GE_API_ENTRY Renderer
  {
    DECLARE_CLASS (Renderer); DECLARE_END;

    int viewX;
    int viewY;
    int viewW;
    int viewH;
    Vector3 back;
    Camera *camera;

  public:
    Renderer();

    void setBackColor(const Vector3 &back);
    void setViewport(int x, int y, int width, int height);
    void setCamera(Camera *camera);
    Camera* getCamera();

    void begin();
    void drawActor(Actor *actor);
    void drawWidget(Widget *w);
    void end();
  };
}

#endif /* __GERENDERER_H */