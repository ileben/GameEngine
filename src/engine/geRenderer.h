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
  class Shader;
  class Material;

  /*
  -----------------------------------------
  Performs scene traversal and rendering
  -----------------------------------------*/

  namespace Deferred
  {
    enum Enum
    {
      Vertex     = 0,
      Normal     = 1,
      Color      = 2,
      Specular   = 3,
      Shadow     = 4
    };
  }

  class GE_API_ENTRY Renderer
  {
    DECLARE_CLASS (Renderer); DECLARE_END;
    
  private:

    int winW;
    int winH;
    int viewX;
    int viewY;
    int viewW;
    int viewH;
    Vector3 back;
    Camera *camera;
    Shader *curShader;
    Material *curMaterial;
    
    bool shadowInit;
    Uint32 shadowMap;
    Uint32 shadowMap2;
    Uint32 shadowFB;

    bool deferredInit;
    Uint32 deferredStencil;
    Uint32 deferredDepth;
    Uint32 deferredAccum;
    Uint32 deferredMaps[4];
    Uint32 deferredFB;
    Uint32 deferredPB;
    Int32 deferredSampler[5];
    Int32 deferredCastShadow;
    Int32 deferredWinSize;
    Shader *shaderGeomFbuf;
    Shader *shaderGeomSkinFbuf;
    Shader *shaderLightSpot;
    
    void updateBuffers ();
    void traverseSceneNoMats (Scene *scene);
    void traverseSceneWithMats (Scene *scene);
    void renderShadowMap (Light *light, Scene *scene);

  public:
    
    Renderer();

    void setWindowSize (int width, int height);
    void setBackColor (const Vector3 &back);
    void setViewport (int x, int y, int width, int height);
    void setCamera (Camera *camera);
    Camera* getCamera();
    
    
    void renderShadowQuad ();
    Shader* getCurrentShader();
    Material* getCurrentMaterial();

    void beginFrame ();
    void renderScene (Scene *scene);
    void renderSceneDeferred (Scene *scene);
    void renderWidget (Widget *w);
    void endFrame ();
  };
}

#endif /* __GERENDERER_H */
