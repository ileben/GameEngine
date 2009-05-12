#ifndef __GERENDERER_H
#define __GERENDERER_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geShaders.h"

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
  Renderer-related enums
  -----------------------------------------*/

  namespace RenderTarget {
    enum Enum
    {
      GBuffer,
      ShadowMap,
      Lighting
    };};

  namespace Deferred {
    enum Enum
    {
      Normal     = 0,
      Color      = 1,
      Specular   = 2,
      Params     = 3,
      Shadow     = 4
    };}

  struct ShaderKey
  {
    RenderTarget::Enum target;
    ClassPtr matClass;
    ClassPtr geomClass;
    Shader *shader;
    
    bool operator == (const ShaderKey &k) const {
      return (target == k.target && matClass == k.matClass && geomClass == k.geomClass);
    }
  };

  #define GE_NUM_GBUFFERS 4
  #define GE_NUM_SAMPLERS 5


  /*
  -----------------------------------------
  Performs scene traversal and rendering
  -----------------------------------------*/

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
    ArrayList< ShaderKey > shaders;
    Float avgLuminance;
    Float maxLuminance;
    
    bool shadowInit;
    Uint32 shadowMap;
    Uint32 shadowMap2;
    Uint32 shadowFB;

    bool deferredInit;
    Uint32 deferredStencil;
    Uint32 deferredDepth;
    Uint32 deferredAccum;
    Uint32 deferredEffects1;
    Uint32 deferredMaps[4];
    Uint32 deferredFB;
    Uint32 deferredPB;
    Int32 deferredSampler[5];
    Int32 deferredCastShadow;
    Int32 deferredWinSize;
    Shader *shaderLightSpot;

    Shader *shaderAmbient;
    Int32 ambientColorSampler;

    Shader *shaderBloom;
    Int32 uBloomColorSampler;
    Int32 uBloomAvgLuminance;
    Int32 uBloomMaxLuminance;

    Uint32 blurFB;
    Uint32 blurMaps[2];

    Shader *shaderBlur;
    Int32 blurColorSampler;
    Int32 uBlurPixelSize;
    Int32 uBlurDirection;
    int blurW, blurH;

    Shader *shaderCell;
    Int32 cellColorSampler;

    Shader *shaderFinal;
    Int32 uFinalColorSampler;
    Int32 uFinalEffectsSampler;
    Int32 uFinalAvgLuminance;
    Int32 uFinalMaxLuminance;
    
    void updateBuffers ();
    void initBuffer (Uint *texID, Uint format, Uint attachment, bool gen=false, int W=-1, int H=-1);
    void traverseSceneNoMats (Scene *scene);
    void traverseSceneWithMats (Scene *scene);
    void renderShadowMap (Light *light, Scene *scene);
    Shader* findShaderByKey (const ShaderKey &key);
    Shader* composeShader (RenderTarget::Enum target,
                           Actor *geometry,
                           Material *material);

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
    void renderWidget (Widget *w);
    void endFrame ();

    void beginDeferred();
    void renderSceneDeferred (Scene *scene);
    void endDeferred();

    void setAvgLuminance (Float l);
    void setMaxLuminance (Float l);
    Float getAvgLuminance ();
    Float getMaxLuminance ();
  };
}

#endif /* __GERENDERER_H */
