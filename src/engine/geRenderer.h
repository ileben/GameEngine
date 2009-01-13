#ifndef __GERENDERER_H
#define __GERENDERER_H

namespace GE
{
  /*
  ------------------------------------------
  Forward declaractions
  ------------------------------------------*/
  
  class Light;

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
    Actor *sceneRoot;
    ArrayList< Light* > sceneLights;

    //Rendering
    void renderActor (Actor *actor);
    void renderActorShadow( Actor *actor );
    
  public:
    
    Renderer();
    
    void setBackColor (const Vector3 &back);
    void setViewport (int x, int y, int width, int height);
    void setCamera (Camera *camera);
    Camera* getCamera();
    
    void renderShadowMap (Light *light, Actor *root);
    void renderShadowQuad ();

    void beginFrame ();
    void beginScene (Actor *root);
    void renderScene ();
    void renderWidget (Widget *w);
    void endScene ();
    void endFrame ();
  };
}

#endif /* __GERENDERER_H */
