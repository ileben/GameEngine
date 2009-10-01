#ifndef __GERENDERER_H
#define __GERENDERER_H

#include "util/geUtil.h"
#include "math/geMath.h"
#include "geShaders.h"
#include "ui/uiUI.h"

namespace GE
{
  /*
  ------------------------------------------
  Forward declaractions
  ------------------------------------------*/
  
  class Actor3D;
  class Camera;
  class Light;
  class Widget;
  class Scene3D;
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

  class Renderer : public Object
  {
    DECLARE_SUBCLASS( Renderer, Object );
    DECLARE_END;
    
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
    Uint32 dofNearBlurMap;
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
    Int32 uAmbientColor;

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

    Shader *shaderDofExtractFar;
    Int32 uDofExtractFarColorSampler;

    Shader *shaderDofExtractNear;
    Int32 uDofExtractNearColorSampler;

    Shader *shaderDofBlurNear;
    Int32 uDofBlurNearColorSampler;
    Int32 uDofBlurNearPixelSize;
    Int32 uDofBlurNearDirection;
    Int32 uDofBlurNearRadius;

    Shader *shaderDofMerge;
    Int32 uDofMergeNearSampler;
    Int32 uDofMergeNearBlurSampler;
    Int32 uDofMergeFarSampler;

    Shader *shaderDofBlur;
    Int32 uDofBlurColorSampler;
    Int32 uDofBlurNearSampler;
    Int32 uDofBlurPixelSize;
    Int32 uDofBlurDirection;
    Int32 uDofBlurRadius;
    Int32 uDofBlurDepthSampler;
    Int32 uDofBlurDofParams;

    Shader *shaderGaussBlur;
    Int32 uGaussBlurColorSampler;
    Int32 uGaussBlurPixelSize;
    Int32 uGaussBlurRadius;

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

    void traverseSceneNoMats (Scene3D *scene);
    void traverseSceneWithMats (Scene3D *scene);
    void renderShadowMap (Light *light, Scene3D *scene);
    Shader* findShaderByKey (const ShaderKey &key);
    Shader* composeShader (RenderTarget::Enum target,
                           Actor3D *geometry,
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
    void renderScene (Scene3D *scene);
    void renderWindow (Scene *w);
    void endFrame ();

    void beginDeferred();
    void renderSceneDeferred (Scene3D *scene);
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
