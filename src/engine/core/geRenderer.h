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
    Class matClass;
    Class geomClass;
    Shader *shader;
    
    bool operator == (const ShaderKey &k) const {
      return (target == k.target && matClass == k.matClass && geomClass == k.geomClass);
    }
  };

  struct LightShaderKey
  {
    Class lightClass;
    Shader *shader;

    bool operator == (const LightShaderKey &k) const {
      return (lightClass == k.lightClass);
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
    CLASS( Renderer, Object,
      4829379e,7482,45e3,8897cee5e9acbf59 );
    
  private:

    int winW;
    int winH;
    int viewX;
    int viewY;
    int viewW;
    int viewH;
    Vector3 back;
    ArrayList< ShaderKey > shaders;
    ArrayList< LightShaderKey > lightShaders;
    Float avgLuminance;
    Float maxLuminance;

    //State
    Camera *curCamera;
    Shader *curShader;
    Material *curMaterial;
    Light *curLight;

    bool fullScreenInit;
    Uint fullScreenVAO;
    Uint fullScreenVBO;
    
    bool shadowInit;
    Uint32 shadowMap;
    Uint32 shadowMap2;
    Uint32 shadowFB;
    Uint shadowMapSize;

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

    Shader *shaderShadow;

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

    void fullScreenQuad ();
    void traverseScene (Scene3D *scene, RenderTarget::Enum target);
    void renderShadowMap (Light *light, Scene3D *scene);
    Shader* findShaderByKey (const ShaderKey &key);

    Shader *findLightShaderByKey (const LightShaderKey &key);
    Shader *getLightShader (Light *light);

    void doToon (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);
    void doDof (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);
    void doBloom (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch);

  public:

    Shader* getShader (RenderTarget::Enum target,
                       Actor3D *geometry,
                       Material *material);
    
    Renderer();

    void setWindowSize (int width, int height);
    Vector2 getWindowSize ();

    void setBackColor (const Vector3 &back);
    void setViewport (int x, int y, int width, int height);
    
    void renderShadowQuad ();
    Shader* getCurrentShader();
    Material* getCurrentMaterial();
    Camera* getCurrentCamera();

    void beginFrame ();
    void renderScene (Scene3D *scene, Camera *camera);
    void renderWindow (Scene *w, Camera *camera);
    void endFrame ();

    void beginDeferred();
    void renderSceneDeferred (Scene3D *scene, Camera *camera);
    void endDeferred();

    void setAvgLuminance (Float l);
    void setMaxLuminance (Float l);
    Float getAvgLuminance ();
    Float getMaxLuminance ();
  };
}

#endif /* __GERENDERER_H */
