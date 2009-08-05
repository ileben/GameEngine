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
    
    bool isDofEnabled;
    Float focusDepth;
    Float focusRange;
    Float nearFalloff;
    Float farFalloff;
    
    bool shadowInit;
    Uint32 shadowMap;
    Uint32 shadowMap2;
    Uint32 shadowFB;

    bool buffersInit;
    Uint32 deferredFB;
    Uint32 deferredStencil;
    Uint32 deferredDepth;
    Uint32 deferredAccum;
    Uint32 deferredMaps[4];
    
    Uint32 dofMap;
    Uint32 dofMedBlurMap;

    Uint32 blurFB;
    Uint32 dofDownMap;
    Uint32 dofNearMap;
    Uint32 depthDownMap;
    Uint32 dofMaxBlurMap;
    Uint32 bloomDownMap;
    Uint32 bloomBlurMap;
    int blurW, blurH;

    Shader *shaderLightSpot;
    Int32 deferredSampler[5];
    Int32 deferredCastShadow;
    Int32 deferredWinSize;

    Shader *shaderAmbient;
    Int32 ambientColorSampler;

    Shader *shaderDofInit;
    Int32 uDofInitColorSampler;
    Int32 uDofInitNormalSampler;
    Int32 uDofInitParamsSampler;
    Int32 uDofInitDofParams;

    Shader *shaderDofDown;
    Int32 uDofDownColorSampler;
    Int32 uDofDownNormalSampler;
    Int32 uDofDownDofParams;
    Int32 uDofDownPixelSize;

    Shader *shaderDofNear;
    Int32 uDofNearNormalSampler;
    Int32 uDofNearDofParams;
    Int32 uDofNearPixelSize;

    Shader *shaderDofBlur;
    Int32 uDofBlurColorSampler;
    Int32 uDofBlurNearSampler;
    Int32 uDofBlurPixelSize;
    Int32 uDofBlurDirection;
    Int32 uDofBlurRadius;
    Int32 uDofBlurDepthSampler;
    Int32 uDofBlurDofParams;

    Shader *shaderDofMix;
    Int32 uDofMixColorSampler;
    Int32 uDofMixMedBlurSampler;
    Int32 uDofMixLargeBlurSampler;
    Int32 uDofMixDepthSampler;
    Int32 uDofMixPixelSize;
    Int32 uDofMixDofParams;

    Shader *shaderBloomDown;
    Int32 uBloomDownColorSampler;
    Int32 uBloomDownAvgLuminance;
    Int32 uBloomDownMaxLuminance;
    Int32 uBloomDownPixelSize;

    Shader *shaderBloomBlur;
    Int32 uBloomBlurColorSampler;
    Int32 uBloomBlurPixelSize;
    Int32 uBloomBlurDirection;
    Int32 uBloomBlurRadius;

    Shader *shaderBloomMix;
    Int32 uBloomMixColorSampler;
    Int32 uBloomMixBloomSampler;
    Int32 uBloomMixAvgLuminance;
    Int32 uBloomMixMaxLuminance;
    
    void initShaders ();
    void initTexture (Uint *texID, Uint format, Uint attachment, bool gen=false, int W=-1, int H=-1);
    void initBuffers ();

    void traverseSceneNoMats (Scene *scene);
    void traverseSceneWithMats (Scene *scene);
    void renderShadowMap (Light *light, Scene *scene);
    Shader* findShaderByKey (const ShaderKey &key);
    Shader* composeShader (RenderTarget::Enum target,
                           Actor *geometry,
                           Material *material);

    void doToon (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);
    void doDof (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);
    void doBloom (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);

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

    void setIsDofEnabled (bool onoff);
    bool getIsDofEnabled ();

    void setDofParams (Float focusDepth, Float focusRange, Float nearFalloff, Float farFalloff );
    void setDofParams (const Vector4 &params);
    Vector4 getDofParams ();
  };
}

#endif /* __GERENDERER_H */
