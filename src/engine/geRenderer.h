#ifndef __GERENDERER_H
#define __GERENDERER_H

#include "util/geUtil.h"
#include "geVectors.h"

namespace GE
{
  /*
  ------------------------------------------
  Forward declaractions
  ------------------------------------------*/
  
  class Actor;
  class Camera;
  class Light;
  class Widget;
  class Scene;

  /*
  -----------------------------------------
  Performs scene traversal and rendering
  -----------------------------------------*/

  class GE_API_ENTRY Renderer
  {
    DECLARE_CLASS (Renderer); DECLARE_END;
    
  private:

    int viewX;
    int viewY;
    int viewW;
    int viewH;
    Vector3 back;
    Camera *camera;
    
    //Scene data
    bool shadowInit;
    Uint32 shadowMap;
    Uint32 shadowMap2;
    Uint32 shadowFB;
    Scene *scene;

    //Rendering
    void renderActor (Actor *actor);
    
  public:
    
    Renderer();
    
    void setBackColor (const Vector3 &back);
    void setViewport (int x, int y, int width, int height);
    void setCamera (Camera *camera);
    Camera* getCamera();
    
    void renderShadowMap (Light *light, Scene *scene);
    void renderShadowQuad ();

    void beginFrame ();
    void beginScene (Scene *scene);
    void renderScene ();
    void renderWidget (Widget *w);
    void endScene ();
    void endFrame ();
  };
}

#endif /* __GERENDERER_H */
