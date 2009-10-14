#include "engine/geRenderer.h"
#include "engine/geCamera.h"
#include "engine/geLight.h"
#include "engine/geShaders.h"
#include "engine/geScene.h"
#include "engine/geShader.h"
#include "widgets/geWidget.h"
#include "engine/geGLHeaders.h"
#include "engine/actors/geSkinMeshActor.h"

#include "engine/embedit/Ambient.embedded"
#include "engine/embedit/Dof.embedded"
#include "engine/embedit/Bloom.embedded"
#include "engine/embedit/Cell.embedded"
#include "engine/embedit/Shadow.embedded"

namespace GE
{
  DEFINE_CLASS (Renderer);
  
  Renderer::Renderer()
  {
    winW = 0;
    winH = 0;
    viewX = 0;
    viewY = 0;
    viewW = 0;
    viewH = 0;
    shadowInit = false;
    buffersInit = false;
    initShaders();

    avgLuminance = 0.5f;
    maxLuminance = 1.0f;

    shadowMapSize = 1024;
    //shadowMapSize = 2048;

    curCamera = NULL;
    curShader = NULL;
    curMaterial = NULL;
  }

  void Renderer::setAvgLuminance (Float l) {
    avgLuminance = l;
  }

  void Renderer::setMaxLuminance (Float l) {
    maxLuminance = l;
  }

  Float Renderer::getAvgLuminance () {
    return avgLuminance;
  }

  Float Renderer::getMaxLuminance () {
    return maxLuminance;
  }

  void Renderer::setBackColor (const Vector3 &color)
  {
    back = color;
  }

  void Renderer::setWindowSize (int width, int height)
  {
    bool change = (width != winW || height != winH);
    
    winW = width;
    winH = height;
    
    if (change) initBuffers();
  }

  Vector2 Renderer::getWindowSize ()
  {
    return Vector2( (Float) winW, (Float) winH );
  }

  /*
  ----------------------------------------------------
  Changes the target rendering area on the GL window
  ----------------------------------------------------*/

  void Renderer::setViewport (int x, int y, int width, int height)
  {
    viewX = x;
    viewY = y;
    viewW = width;
    viewH = height;
  }

  /*
  --------------------------------------
  Rendering interface
  --------------------------------------*/

  Shader* Renderer::findLightShaderByKey (const LightShaderKey &key)
  {
    for (UintSize s=0; s<lightShaders.size(); ++s)
      if (lightShaders[s] == key)
        return lightShaders[s].shader;

    return NULL;
  }

  Shader* Renderer::getLightShader (Light *light)
  {
    LightShaderKey key;
    key.lightClass = ClassOf( light );

    Shader *shader = findLightShaderByKey( key );
    if (shader != NULL) return shader;


    printf( "\n");
    printf( "==============================\n");
    printf( "    Composing Light Shader    \n" );
    printf( "==============================\n");
    printf( "Light: %s\n", key.lightClass->getString() );
    printf( "\n" );

    shader = new Shader();
    light->composeShader( shader );
    shader->link();

    key.shader = shader;
    lightShaders.pushBack( key );

    return shader;
  }

  Shader* Renderer::findShaderByKey (const ShaderKey &key)
  {
    for (UintSize s=0; s<shaders.size(); ++s)
      if (shaders[s] == key)
        return shaders[s].shader;

    return NULL;
  }

  Shader* Renderer::getShader (RenderTarget::Enum target,
                               Actor3D *geometry,
                               Material *material)
  {
    ShaderKey key;
    key.target = target;
    
    if (geometry == NULL) key.geomClass = Class(Actor3D);
    else key.geomClass = geometry->getShaderComposingClass();
    
    if (material == NULL) key.matClass = Class(Material);
    else key.matClass = material->getShaderComposingClass();

    Shader* shader = findShaderByKey( key );
    if (shader != NULL) {
      curShader = shader;
      return shader;
    }

    printf( "\n");
    printf( "=================================\n" );
    printf( "    Composing Geometry Shader    \n" );
    printf( "=================================\n" );
    printf( "Target: %s\n", target == RenderTarget::GBuffer ? "GBuffer" : "ShadowMap" );
    printf( "Geometry: %s\n", key.geomClass->getString() );
    printf( "Material: %s\n", key.matClass->getString() );
    printf( "\n" );

    shader = new Shader;
    if (geometry != NULL) geometry->composeShader( shader );
    if (material != NULL) material->composeShader( shader );
    shader->compose( target );
    
    key.shader = shader;
    shaders.pushBack( key );

    curShader = shader;
    return shader;
  }

  Shader* Renderer::getCurrentShader() {
    return curShader;
  }
  
  Material* Renderer::getCurrentMaterial() {
    return curMaterial;
  }

  Camera* Renderer::getCurrentCamera() {
    return curCamera;
  }

  void Renderer::beginFrame()
  {
    //Clear the framebuffer
    glClearColor (back.x, back.y, back.z, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::endFrame()
  {
    glutSwapBuffers ();
  }

  void Renderer::renderShadowQuad ()
  {
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    GLProgram::UseFixed();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glEnable( GL_COLOR_MATERIAL );
    glColor3f( 1.0, 1.0, 1.0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, shadowMap2 );
    glEnable( GL_TEXTURE_2D );
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
    glTexCoord2f( 1, 0 ); glVertex2f( 100, 0 );
    glTexCoord2f( 1, 1 ); glVertex2f( 100, 100 );
    glTexCoord2f( 0, 1 ); glVertex2f( 0, 100 );
    glEnd();
    
    glDisable( GL_TEXTURE_2D );
  }

  void Renderer::initShaders ()
  {
    shaderShadow = new Shader;
    shaderShadow->fromString( Shadow_VS, Shadow_FS );

    shaderAmbient = new Shader;
    shaderAmbient->registerUniform( ShaderType::Fragment, DataUnit::Vec3, "ambientColor" );
    shaderAmbient->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderAmbient->fromString( Ambient_VertexSource, Ambient_FragmentSource );
    uAmbientColor = shaderAmbient->getUniformID( "ambientColor" );
    ambientColorSampler = shaderAmbient->getUniformID( "samplerColor" );

    shaderDofInit = new Shader;
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerParams" );
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Vec3, "dofParams" );
    shaderDofInit->compile( ShaderType::Vertex, Dof_VS );
    shaderDofInit->compile( ShaderType::Fragment, ComputeCoC_Func );
    shaderDofInit->compile( ShaderType::Fragment, QuantizeLight_Func );
    shaderDofInit->compile( ShaderType::Fragment, DofInit_FS );
    shaderDofInit->link();
    uDofInitColorSampler = shaderDofInit->getUniformID( "samplerColor" );
    uDofInitNormalSampler = shaderDofInit->getUniformID( "samplerNormal" );
    uDofInitParamsSampler = shaderDofInit->getUniformID( "samplerParams" );
    uDofInitDofParams = shaderDofInit->getUniformID( "dofParams" );

    shaderDofDown = new Shader;
    shaderDofDown->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofDown->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shaderDofDown->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDofDown->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofDown->compile( ShaderType::Vertex, Dof_VS );
    shaderDofDown->compile( ShaderType::Fragment, ComputeCoC_Func );
    shaderDofDown->compile( ShaderType::Fragment, DofDown_FS );
    shaderDofDown->link();
    uDofDownColorSampler = shaderDofDown->getUniformID( "samplerColor" );
    uDofDownNormalSampler = shaderDofDown->getUniformID( "samplerNormal" );
    uDofDownPixelSize = shaderDofDown->getUniformID( "pixelSize" );
    uDofDownDofParams = shaderDofDown->getUniformID( "dofParams" );

    shaderDofExtractFar = new Shader;
    shaderDofExtractFar->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofExtractFar->fromString( Dof_VS, DofExtractFar_FS );
    uDofExtractFarColorSampler = shaderDofExtractFar->getUniformID( "samplerColor" );

    shaderDofExtractNear = new Shader;
    shaderDofExtractNear->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofExtractNear->fromString( Dof_VS, DofExtractNear_FS );
    uDofExtractNearColorSampler = shaderDofExtractNear->getUniformID( "samplerColor" );

    shaderDofBlurNear = new Shader;
    shaderDofBlurNear->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofBlurNear->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDofBlurNear->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderDofBlurNear->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderDofBlurNear->fromString( Dof_VS, DofBlurNear_FS );
    uDofBlurNearSampler = shaderDofBlurNear->getUniformID( "samplerColor" );
    uDofBlurNearPixelSize = shaderDofBlurNear->getUniformID( "pixelSize" );
    uDofBlurNearDirection = shaderDofBlurNear->getUniformID( "direction" );
    uDofBlurNearRadius = shaderDofBlurNear->getUniformID( "radius" );

    shaderDofMerge = new Shader;
    shaderDofMerge->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNear" );
    shaderDofMerge->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNearBlur" );
    shaderDofMerge->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerFar" );
    shaderDofMerge->fromString( Dof_VS, DofMerge_FS );
    uDofMergeNearSampler = shaderDofMerge->getUniformID( "samplerNear" );
    uDofMergeNearBlurSampler = shaderDofMerge->getUniformID( "samplerNearBlur" );
    uDofMergeFarSampler = shaderDofMerge->getUniformID( "samplerFar" );

    shaderGaussBlur = new Shader;
    shaderGaussBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderGaussBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderGaussBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderGaussBlur->fromString( Dof_VS, GaussBlur_FS );
    uGaussBlurColorSampler = shaderGaussBlur->getUniformID( "samplerColor" );
    uGaussBlurPixelSize = shaderGaussBlur->getUniformID( "pixelSize" );
    uGaussBlurRadius = shaderGaussBlur->getUniformID( "radius" );

    shaderDofBlur = new Shader;
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNear" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepth" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofBlur->fromString( Dof_VS, DofBlur_FS );
    uDofBlurColorSampler = shaderDofBlur->getUniformID( "samplerColor" );
    uDofBlurNearSampler = shaderDofBlur->getUniformID( "samplerNear" );
    uDofBlurPixelSize = shaderDofBlur->getUniformID( "pixelSize" );
    uDofBlurDirection = shaderDofBlur->getUniformID( "direction" );
    uDofBlurRadius = shaderDofBlur->getUniformID( "radius" );
    uDofBlurDepthSampler = shaderDofBlur->getUniformID( "samplerDepth" );
    uDofBlurDofParams = shaderDofBlur->getUniformID( "dofParams" );

    shaderDofMix = new Shader;
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerMedBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerLargeBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepth" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepthBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofMix->compile( ShaderType::Vertex, Dof_VS );
    shaderDofMix->compile( ShaderType::Fragment, ComputeCoC_Func );
    shaderDofMix->compile( ShaderType::Fragment, DofMix_FS );
    shaderDofMix->link();
    uDofMixColorSampler = shaderDofMix->getUniformID( "samplerColor" );
    uDofMixMedBlurSampler = shaderDofMix->getUniformID( "samplerMedBlur" );
    uDofMixLargeBlurSampler = shaderDofMix->getUniformID( "samplerLargeBlur" );
    uDofMixDepthSampler = shaderDofMix->getUniformID( "samplerDepth" );
    uDofMixDofParams = shaderDofMix->getUniformID( "dofParams" );

    shaderBloomDown = new Shader;
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Float, "avgLuminance" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Float, "maxLuminance" );
    shaderBloomDown->compile( ShaderType::Vertex, Bloom_VS );
    shaderBloomDown->compile( ShaderType::Fragment, ToneMap_Func );
    shaderBloomDown->compile( ShaderType::Fragment, BloomDown_FS );
    shaderBloomDown->link();
    uBloomDownColorSampler = shaderBloomDown->getUniformID( "samplerColor" );
    uBloomDownPixelSize = shaderBloomDown->getUniformID( "pixelSize" );
    uBloomDownAvgLuminance = shaderBloomDown->getUniformID( "avgLuminance" );
    uBloomDownMaxLuminance = shaderBloomDown->getUniformID( "maxLuminance" );

    shaderBloomBlur = new Shader;
    shaderBloomBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBloomBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderBloomBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderBloomBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderBloomBlur->fromString( Bloom_VS, BloomBlur_FS );
    uBloomBlurColorSampler = shaderBloomBlur->getUniformID( "samplerColor" );
    uBloomBlurPixelSize = shaderBloomBlur->getUniformID( "pixelSize" );
    uBloomBlurDirection = shaderBloomBlur->getUniformID( "direction" );
    uBloomBlurRadius = shaderBloomBlur->getUniformID( "radius" );

    shaderBloomMix = new Shader;
    shaderBloomMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBloomMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerBloom" );
    shaderBloomMix->registerUniform( ShaderType::Fragment, DataUnit::Float, "avgLuminance" );
    shaderBloomMix->registerUniform( ShaderType::Fragment, DataUnit::Float, "maxLuminance" );
    shaderBloomMix->compile( ShaderType::Vertex, Bloom_VS );
    shaderBloomMix->compile( ShaderType::Fragment, ToneMap_Func );
    shaderBloomMix->compile( ShaderType::Fragment, BloomMix_FS );
    shaderBloomMix->link();
    uBloomMixColorSampler = shaderBloomMix->getUniformID( "samplerColor" );
    uBloomMixBloomSampler = shaderBloomMix->getUniformID( "samplerBloom" );
    uBloomMixAvgLuminance = shaderBloomMix->getUniformID( "avgLuminance" );
    uBloomMixMaxLuminance = shaderBloomMix->getUniformID( "maxLuminance" );

    glUseProgram( 0 );
  }

  void Renderer::initTexture (Uint *texID, Uint format, Uint attachment, bool gen, int W, int H)
  {
    if (W == -1) W = winW;
    if (H == -1) H = winH;
    if (gen) glGenTextures( 1, texID );
    glBindTexture( GL_TEXTURE_2D, *texID );
    glTexImage2D( GL_TEXTURE_2D, 0, format, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *texID, 0 );
  }

  void Renderer::initBuffers ()
  {
    //Destroy old buffers if present
    if (buffersInit)
    {
      glDeleteFramebuffers( 1, &deferredFB );
      //glDeleteRenderbuffers( 1, &deferredDepth );
      glDeleteTextures( 1, &deferredDepth );
      glDeleteTextures( GE_NUM_GBUFFERS, deferredMaps );
      glDeleteTextures( 1, &deferredAccum );
      glDeleteTextures( 1, &dofMap );
      glDeleteTextures( 1, &dofMedBlurMap );

      glDeleteFramebuffers( 1, &blurFB );
      glDeleteTextures( 1, &dofDownMap );
      glDeleteTextures( 1, &dofNearMap );
      glDeleteTextures( 1, &depthDownMap );
      glDeleteTextures( 1, &dofMaxBlurMap );
    };

    //Generate deferred framebuffer
    glGenFramebuffers( 1, &deferredFB );
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    //Generate deferred depth-stencil buffer
    /*
    glGenRenderbuffers( 1, &deferredDepth );
    glBindRenderbuffer( GL_RENDERBUFFER, deferredDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winW, winH );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );
    */
    glGenTextures( 1, &deferredDepth );
    glBindTexture( GL_TEXTURE_2D, deferredDepth );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, winW, winH, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferredDepth, 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, deferredDepth, 0 );

    //Generate deferred geometry buffers
    glGenTextures( GE_NUM_GBUFFERS, deferredMaps );
    initTexture( &deferredMaps[ Deferred::Normal ], GL_RGBA16F, GL_COLOR_ATTACHMENT1 );
    initTexture( &deferredMaps[ Deferred::Color ], GL_RGBA8, GL_COLOR_ATTACHMENT2 );
    initTexture( &deferredMaps[ Deferred::Specular ], GL_RGBA8, GL_COLOR_ATTACHMENT3 );
    initTexture( &deferredMaps[ Deferred::Params ], GL_RGBA16F, GL_COLOR_ATTACHMENT4 );

    //Generate deferred lighting accumulation buffer (linear filter for downsampling)
    initTexture( &deferredAccum, GL_RGBA16F, GL_COLOR_ATTACHMENT0, true );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Generate dof buffer (linear filter for downsampling)
    initTexture( &dofMap, GL_RGBA16F, GL_COLOR_ATTACHMENT5, true );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Generate medium blur buffer
    initTexture( &dofMedBlurMap, GL_RGBA16F, GL_COLOR_ATTACHMENT6, true );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Check framebuffer status
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if (status != GL_FRAMEBUFFER_COMPLETE)
      printf( "Deferred framebuffer INCOMPLETE! (status: 0x%x)\n", (int)status );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    //Generate blur buffers
    blurW = winW / 4;
    blurH = winH / 4;

    glGenFramebuffers( 1, &blurFB );
    glBindFramebuffer( GL_FRAMEBUFFER, blurFB );

    initTexture( &depthDownMap, GL_RGBA16F, GL_COLOR_ATTACHMENT0, true, blurW, blurH );
    initTexture( &dofDownMap, GL_RGBA16F, GL_COLOR_ATTACHMENT1, true, blurW, blurH );
    initTexture( &dofNearMap, GL_RGBA16F, GL_COLOR_ATTACHMENT2, true, blurW, blurH );

    initTexture( &dofNearBlurMap, GL_RGBA16F, GL_COLOR_ATTACHMENT6, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    initTexture( &dofMaxBlurMap, GL_RGBA16F, GL_COLOR_ATTACHMENT3, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    initTexture( &bloomDownMap, GL_RGBA16F, GL_COLOR_ATTACHMENT4, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    initTexture( &bloomBlurMap, GL_RGBA16F, GL_COLOR_ATTACHMENT5, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Check framebuffer status
    status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if (status != GL_FRAMEBUFFER_COMPLETE)
      printf( "Blur framebuffer INCOMPLETE! (status: 0x%x)\n", (int)status );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    buffersInit = true;
  }

  void Renderer::renderShadowMap (Light *light, Scene3D *scene)
  {
    Uint S = shadowMapSize;
    if (!shadowInit)
    {
      /*
      glGenTextures( 1, &shadowMap2 );
      glBindTexture( GL_TEXTURE_2D, shadowMap2 );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, S, S, 0, GL_LUMINANCE, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      */
      glGenTextures( 1, &shadowMap );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      
      glGenFramebuffers( 1, &shadowFB );
      glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0 );
      glDrawBuffer( GL_NONE );
      glReadBuffer( GL_NONE );

      shadowInit = true;
    }

    //Setup GL state to render to shadow texture
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_STENCIL_TEST );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 5.0f, 2.0f );
    
    glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
    glClear( GL_DEPTH_BUFFER_BIT );
    
    //Setup view from the lights perspective
    glViewport( 0, 0, S, S );

    Matrix4x4 lightProj = light->getProjection();
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( (GLfloat*) lightProj.m );
    
    Matrix4x4 lightView = light->getGlobalMatrix().affineNormalize().affineInverse();
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat*) lightView.m );

    //Render scene
    traverseScene( scene, RenderTarget::ShadowMap );

    //Restore state
    glDisable( GL_POLYGON_OFFSET_FILL );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );


    ////////////////////////////////////////////////////////
    /*
    GLfloat *pixels = new GLfloat[ S * S ];

    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glBindTexture( GL_TEXTURE_2D, shadowMap );
    glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels );

    float lastp = -1.0f;
    for (int y=0; y<S; ++y)
    {
      for (int x=0; x<S; ++x)
      {
        float p = pixels[ y * S + x ];
        //if (lastp > 0.0f && p != lastp)
          //printf( "p: %f\n", p );
        //lastp = p;
        float pp = (p - 0.95f) / 0.05f;
        pixels[ y * S + x ] = pp;
        //pixels[ y * 512 + x ] = 0.2f;
      }
    }
    
    glBindTexture( GL_TEXTURE_2D, shadowMap2 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, S, S, GL_LUMINANCE, GL_FLOAT, pixels );

    delete[] pixels;*/
  }

  void fullScreenQuad ()
  {
    const GLfloat texCoords[] = {
      0,0,
      1,0,
      1,1,
      0,1 };

    const GLfloat vertCoords[] = {
      -1,-1,1,
      +1,-1,1,
      +1,+1,1,
      -1,+1,1 };

    //glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
    glVertexPointer( 3, GL_FLOAT, 0, vertCoords );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glEnableClientState( GL_VERTEX_ARRAY );
    
    glDrawArrays( GL_QUADS, 0, 4 );
    
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
  }

  void Renderer::beginDeferred()
  {
    //Clear the lighting accumulation texture
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    //GLenum drawBuffers[] = {
    //  GL_COLOR_ATTACHMENT0,
    //  GL_COLOR_ATTACHMENT5 };
    //glDrawBuffers( 2, drawBuffers );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
  }

  void Renderer::renderSceneDeferred (Scene3D *scene, Camera *camera)
  {
    if (camera == NULL)
      return;

    //Set camera current
    curCamera = camera;

    //Update scene
    if (scene->hasChanged())
      scene->updateChanges();

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    camera->updateProjection( (Float) viewW, (Float) viewH );
    camera->updateView();

    //Render into geometry textures
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    //Clear depth (normal buffer alpha) with far clip values
    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    glClearColor( 0, 0, 0, camera->getFarClipPlane() );
    glClear( GL_COLOR_BUFFER_BIT );

    //Clear the rest of the buffers
    GLenum clearBuffers[] = {
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
      GL_COLOR_ATTACHMENT4 };
    glDrawBuffers( 3, clearBuffers );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Enable all buffers for drawing
    GLenum drawBuffers[] = {
      GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
      GL_COLOR_ATTACHMENT4 };
    glDrawBuffers( 4, drawBuffers );
    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( viewX, viewY, viewW, viewH );

    glEnable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );

    traverseScene( scene, RenderTarget::GBuffer );

    //Ambient light pass
    shaderAmbient->use();
    glUniform3fv( uAmbientColor, 1, (GLfloat*) &scene->getAmbientColor() );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Color ] );
    glEnable( GL_TEXTURE_2D );

    glDisable( GL_BLEND );
    glDisable( GL_LIGHTING );
    glDisable( GL_CULL_FACE );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_GREATER );
    glDepthMask( GL_FALSE );

    fullScreenQuad();

    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LESS );
    glDisable( GL_TEXTURE_2D );

    static int lastVisibleLights = -1;
    int numVisibleLights = 0;

    UintSize numLights = scene->getLights()->size();
    GLuint *lightQueries = new GLuint[ numLights ];
    glGenQueries( (GLsizei) numLights, lightQueries );


    ///////////////////////////////////////////////////////////////
    //Perform shading for each light
    UintSize stencilIndex = 0;
    int numStencilBits = 8;

    //Walk all the lights in the scene
    for (UintSize l=0; l<numLights; ++l)
    {
      /////////////////////////////////////////////////////////////////
      //Preset stencil for as many lights in advance as there is bits

      UintSize lastStencilIndex = Util::Min( l + numStencilBits, numLights );
      for (; stencilIndex < lastStencilIndex; stencilIndex++)
      {
        //The stencil bit being set for this light
        int stencilMask = (1 << (stencilIndex % numStencilBits));
        Light* light = scene->getLights()->at( stencilIndex );
      
        //Only write to stencil buffer
        glUseProgram( 0 );
        glDrawBuffer( GL_NONE );
        glDepthMask( GL_FALSE );
        glDisable( GL_BLEND );
        glDisable( GL_LIGHTING );
        glEnable( GL_CULL_FACE );
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_STENCIL_TEST );
        glStencilMask( stencilMask );

        //Setup view and projection
        glViewport( viewX, viewY, viewW, viewH );
        camera->updateProjection( (Float) viewW, (Float) viewH );
        camera->updateView();

        //Add light's world matrix
        Matrix4x4 worldCtm = light->getGlobalMatrix().affineNormalize();
        glMatrixMode( GL_MODELVIEW );
        glMultMatrixf( (GLfloat*) worldCtm.m );

        //Avoid front faces being clipped by the camera near plane
        Vector3 worldEye = camera->getGlobalMatrix().affineNormalize() * Vector3(0,0,0);
        if (light->isPointInVolume( worldEye, camera->getNearClipPlane()*2 ))
        {
          //Pass for pixels in front of light volume back
          glDepthFunc( GL_GEQUAL );
          glStencilFunc( GL_ALWAYS, 0xFF, stencilMask );
          glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

          //Render light volume back faces and query
          glCullFace( GL_FRONT );
          glBeginQuery( GL_SAMPLES_PASSED, lightQueries[ stencilIndex ] );
          light->renderVolume();
          glEndQuery( GL_SAMPLES_PASSED );
        }
        else
        {
          //Pass for pixels in front of light volume back and incr stencil
          glDepthFunc( GL_GEQUAL );
          glStencilFunc( GL_ALWAYS, 0xFF, stencilMask );
          glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

          //Render light volume back faces
          glCullFace( GL_FRONT );
          light->renderVolume();

          //Pass for pixels behind light volume front and in front of light volume back
          glDepthFunc( GL_LESS );
          glStencilFunc( GL_EQUAL, 0xFF, stencilMask );
          glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

          //Render light volume front faces and query
          glCullFace( GL_BACK );
          glBeginQuery( GL_SAMPLES_PASSED, lightQueries[ stencilIndex ] );
          light->renderVolume();
          glEndQuery( GL_SAMPLES_PASSED );
        }

        //Restore other state
        glDisable( GL_STENCIL_TEST );
        glDepthMask( GL_TRUE );
        glDepthFunc( GL_LESS );
        glCullFace( GL_BACK );
      }

      //////////////////////////////////////////////////////////////////
      //Render the shadow map

      //The stencil bit used by this light
      int stencilMask = (1 << (l % numStencilBits));
      Light *light = scene->getLights()->at( l );
      curLight = light;

      //Obtain light query result
      GLint litSamples = 0;
      glGetQueryObjectiv( lightQueries[l], GL_QUERY_RESULT, &litSamples );
      if (litSamples > 0) numVisibleLights++;
      else continue;

      //Check if shadows enabled and render
      if (light->getCastShadows())
        renderShadowMap( light, scene );

      /////////////////////////////////////////////////
      //Finally light the pixels that need to be lit

      Shader *shader = getLightShader( light );
      shader->use();

      //Setup view and projection
      glViewport( viewX, viewY, viewW, viewH );
      camera->updateProjection( (Float) viewW, (Float) viewH );
      camera->updateView();
      
      //Add light's world matrix and enable it
      Matrix4x4 worldCtm = light->getGlobalMatrix().affineNormalize();
      glMatrixMode( GL_MODELVIEW );
      glMultMatrixf( (GLfloat*) worldCtm.m );
      light->enable( 0 );

      //Render to accumulation texture
      glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
      glDrawBuffer( GL_COLOR_ATTACHMENT0 );
      glDisable( GL_DEPTH_TEST );
      glEnable( GL_STENCIL_TEST );
      glEnable( GL_CULL_FACE );
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE );

      glStencilMask( stencilMask );
      glStencilFunc( GL_EQUAL, 0xFF, stencilMask );
      glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );

      //Setup camera-eye to light-clip matrix
      Matrix4x4 cam = camera->getGlobalMatrix().affineNormalize();
      Matrix4x4 lightProj = light->getProjection();
      Matrix4x4 lightInv = light->getGlobalMatrix().affineNormalize().affineInverse();
      Matrix4x4 tex = lightProj * lightInv * cam;
      glActiveTexture( GL_TEXTURE0 );
      glMatrixMode( GL_TEXTURE );
      glLoadMatrixf( (GLfloat*) tex.m );
      
      //Render light volume back faces
      glCullFace( GL_FRONT );

      light->begin( shader, Vector2( (Float)winW, (Float)winH ), deferredMaps, shadowMap );
      light->renderVolume();
      light->end();

      //Restore state
      glDisable( GL_STENCIL_TEST );
      glCullFace( GL_BACK );
      
      /*
      //Draw light volume
      glUseProgram( 0 );
      glDisable( GL_DEPTH_TEST );
      glEnable( GL_CULL_FACE );
      //glDisable( GL_CULL_FACE );
      glDisable( GL_BLEND );
      glDisable( GL_LIGHTING );
      glEnable( GL_COLOR_MATERIAL );
      glColor3f( 0,0,1 );
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      light->renderVolume();

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      */
    }

    glDeleteQueries( (GLsizei) numLights, lightQueries );
    delete[] lightQueries;
/*
    if (numVisibleLights != lastVisibleLights) {
      printf( "numVisibleLights: %d\n", numVisibleLights );
      lastVisibleLights = numVisibleLights;
    }*/
  }

  void Renderer::doToon (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch)
  {
    DofParams dofParams = ((Camera3D*)curCamera)->getDofParams();
    float focusZ = dofParams.focusCenter;
    float focusW = dofParams.focusRange;
    float farW = dofParams.falloffFar;
    float nearW = dofParams.falloffNear;

    ///////////////////////////////////////////////////
    //Apply toon shading and initialize CoC values.
    //Render target: dofMap

    glBindFramebuffer( GL_FRAMEBUFFER, targetFB );
    if (targetFB != 0) glDrawBuffer( targetAtch );

    shaderDofInit->use();
    glUniform4f( uDofInitDofParams, focusZ, focusW, nearW, farW );

    glUniform1i( uDofInitColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofInitNormalSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ]);
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofInitParamsSampler, 2 );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Params ] );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();
  }

  void Renderer::doDof (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch)
  {
    DofParams dofParams = ((Camera3D*)curCamera)->getDofParams();
    float focusZ = dofParams.focusCenter;
    float focusW = dofParams.focusRange;
    float farW = dofParams.falloffFar;
    float nearW = dofParams.falloffNear;

    int medBlurRadius = 5;
    int maxBlurRadius = 5;

    ////////////////////////////////////////////
    //Blur with medium size radius.
    //Render target: dofMidBlurMap
/*
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
    glDrawBuffer( GL_COLOR_ATTACHMENT6 );

    shaderDofBlur->use();
    glUniform1i( uDofBlurRadius, medBlurRadius );
    glUniform2f( uDofBlurPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uDofBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();
  */  

    ////////////////////////////////////////////
    //Downsample depth information.
    //Render target: depthDownMap

    glViewport( 0,0,blurW,blurH );
    glBindFramebuffer( GL_FRAMEBUFFER, blurFB );
/*
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    shaderDofDown->use();
    glUniform2f( uDofDownPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uDofDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();
*/
    /////////////////////////////////////////////
    //Downsample color and CoC information.
    //Render target: dofDownMap

    glDrawBuffer( GL_COLOR_ATTACHMENT1 );

    shaderDofDown->use();
    glUniform2f( uDofDownPixelSize, 1.0f/winW, 1.0f/winH );
    glUniform4f( uDofDownDofParams, focusZ, focusW, nearW, farW );

    glUniform1i( uDofDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofDownNormalSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();

    /////////////////////////////////////////////
    //Extract far CoC
    //Render target: depthDownMap

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    shaderDofExtractFar->use();

    glUniform1i( uDofExtractFarColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofDownMap );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();

    /////////////////////////////////////////////
    //Extract near CoC
    //Render target: dofNearMap

    glDrawBuffer( GL_COLOR_ATTACHMENT2 );

    shaderDofExtractNear->use();

    glUniform1i( uDofExtractNearColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofDownMap );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();

    ///////////////////////////////////////////////////
    //Blur dofNearMap in X direction
  
    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );

    shaderDofBlurNear->use();
    glUniform2f( uDofBlurNearPixelSize, 1.0f/blurW, 1.0f/blurH);
    glUniform2f( uDofBlurNearDirection, 1.0, 0.0 );
    glUniform1i( uDofBlurNearRadius, maxBlurRadius );

    glUniform1i( uDofBlurNearColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofNearMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

    ///////////////////////////////////////////////////
    //Blur dofNearMap in Y direction

    glDrawBuffer( GL_COLOR_ATTACHMENT6 );

    glUniform2f( uDofBlurNearDirection, 0.0, 1.0 );

    glUniform1i( uDofBlurNearColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofDownMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();


    /////////////////////////////////////////////
    //Merge near and far CoC into alpha chanel
    //Render target: dofDownMap

    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );

    shaderDofMerge->use();

    glUniform1i( uDofMergeNearSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofNearMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMergeNearBlurSampler, 2 );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, dofNearBlurMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMergeFarSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, depthDownMap );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );


    /////////////////////////////////////////////////////////////
    //Blur downsampled color with large blur radius
    //Render target: dofMaxBlurMap
  
    glDrawBuffer( GL_COLOR_ATTACHMENT3 );

    shaderDofBlur->use();
    glUniform1i( uDofBlurRadius, maxBlurRadius );
    glUniform2f( uDofBlurPixelSize, 1.0f/blurW, 1.0f/blurH);
    glUniform4f( uDofBlurDofParams, focusZ, focusW, nearW, farW );

    glUniform1i( uDofBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofDownMap );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();

    /////////////////////////////////////////////////
    //Mix blurred and original DoF images
    //Render target: input argument

    glViewport( viewX, viewY, viewW, viewH);
    glBindFramebuffer( GL_FRAMEBUFFER, targetFB );
    if (targetFB != 0) glDrawBuffer( targetAtch );

    shaderDofMix->use();
    glUniform4f( uDofMixDofParams, focusZ, focusW, nearW, farW );

    glUniform1i( uDofMixColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixMedBlurSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    //glBindTexture( GL_TEXTURE_2D, dofMedBlurMap );
    glBindTexture( GL_TEXTURE_2D, dofNearBlurMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixLargeBlurSampler, 2 );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, dofMaxBlurMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixDepthSampler, 3 );
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad ();
  }

  void Renderer::doBloom (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch)
  {
    int bloomBlurRadius = 14;

    //////////////////////////////////////////////////////////
    //Downsample, tone map and bloom cutoff to bloomDownMap

    glViewport( 0,0,blurW,blurH );
    glBindFramebuffer( GL_FRAMEBUFFER, blurFB );
    glDrawBuffer( GL_COLOR_ATTACHMENT4 );

    shaderBloomDown->use();
    glUniform1f( uBloomDownAvgLuminance, avgLuminance );
    glUniform1f( uBloomDownMaxLuminance, maxLuminance );
    glUniform2f( uBloomDownPixelSize, 1.0f/winW, 1.0f/winH );

    glUniform1i( uBloomDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    ///////////////////////////////////////////////////
    //Blur bloomDownMap in X direction to bloomBlurMap
  
    glDrawBuffer( GL_COLOR_ATTACHMENT5 );

    shaderBloomBlur->use();
    glUniform2f( uBloomBlurPixelSize, 1.0f/blurW, 1.0f/blurH);
    glUniform2f( uBloomBlurDirection, 1.0, 0.0 );
    glUniform1i( uBloomBlurRadius, bloomBlurRadius );

    glUniform1i( uBloomBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, bloomDownMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    ///////////////////////////////////////////////////
    //Blur bloomBlurMap in Y direction to bloomDownMap

    glDrawBuffer( GL_COLOR_ATTACHMENT4 );

    glUniform2f( uBloomBlurDirection, 0.0, 1.0 );

    glUniform1i( uBloomBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, bloomBlurMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    ////////////////////////////////////////

    glViewport( viewX, viewY, viewW, viewH);
    glBindFramebuffer( GL_FRAMEBUFFER, targetFB );
    if (targetFB != 0) glDrawBuffer( targetAtch );

    shaderBloomMix->use();
    glUniform1f( uBloomMixAvgLuminance, avgLuminance );
    glUniform1f( uBloomMixMaxLuminance, maxLuminance );

    glUniform1i( uBloomMixColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uBloomMixBloomSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, bloomDownMap );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad ();
  }

  void Renderer::endDeferred()
  {
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glColor3f( 1,1,1 );

    doToon( deferredAccum, deferredFB, GL_COLOR_ATTACHMENT5 );
    
    if (((Camera3D*)curCamera)->getDofEnabled())
    {
      doDof( dofMap, deferredFB, GL_COLOR_ATTACHMENT0 );
      doBloom( deferredAccum, 0, 0 );
    }
    else
    {
      doBloom( dofMap, 0, 0 );
    }

    glActiveTexture( GL_TEXTURE4);
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE3);
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE2);
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE1);
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE0);
    glDisable( GL_TEXTURE_2D );
  }

  void Renderer::renderScene (Scene3D *scene, Camera *camera)
  {
    if (camera == NULL)
      return;

    //Set camera current
    curCamera = camera;

    //Update scene
    if (scene->hasChanged())
      scene->updateChanges();

    //Render shadow map for the first light
    if (!scene->getLights()->empty())
      renderShadowMap( scene->getLights()->first(), scene );

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    camera->updateProjection( (Float) viewW, (Float) viewH );
    camera->updateView();

    //Setup camera-eye to light-clip matrix
    if (!scene->getLights()->empty())
    {
      Light *l = scene->getLights()->first();
      
      Matrix4x4 cam = camera->getGlobalMatrix().affineNormalize();
      Matrix4x4 lightProj = l->getProjection();
      Matrix4x4 lightInv = l->getGlobalMatrix().affineNormalize().affineInverse();
      Matrix4x4 tex = lightProj * lightInv * cam;
      
      glMatrixMode( GL_TEXTURE );
      glLoadMatrixf( (GLfloat*) tex.m );

      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
    }
    
    //Enable lights
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
    {
      Light *light = scene->getLights()->at( l );

      //Setup transformation matrix
      Matrix4x4 worldCtm = light->getGlobalMatrix().affineNormalize();
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glMultMatrixf( (GLfloat*) worldCtm.m );
      
      //Setup light data
      light->enable( (int)l );
      
      glPopMatrix();
    }

    //Render geometry with materials
    traverseScene( scene, RenderTarget::GBuffer );

    //Disable lights
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
      glDisable( GL_LIGHT0 + (int) l );
  }

  /*
  ------------------------------------------------------
  Renderer class has authority over the rendering steps
  invoked and their order. This is to allow for special
  rendering passes - e.g. when a shadow map is being
  rendered using materials (and possibly enabling other
  shading programs) might not be desired.
  ------------------------------------------------------*/

  class Frustum
  {
  public:

    enum Result
    {
      Outside,
      Inside
    };

    enum PlaneIndex
    {
      Left   = 0,
      Right  = 1,
      Bottom = 2,
      Top    = 3,
      Near   = 4,
      Far    = 5
    };

    Vector4 planes[6];

    Result testPoint (const Vector3 &p, int pl) const
    {
      if (p.x  * planes[pl].x +
          p.y  * planes[pl].y +
          p.z  * planes[pl].z +
          1.0f * planes[pl].w > 0.0f)
        return Inside;
      else return Outside;
    }
  };

  void getFrustum (const Matrix4x4 &m, Frustum &outFrustum)
  {
    outFrustum.planes[ Frustum::Left ]   = m.getRow(3) + m.getRow(0);
    outFrustum.planes[ Frustum::Right ]  = m.getRow(3) - m.getRow(0);
    outFrustum.planes[ Frustum::Bottom ] = m.getRow(3) + m.getRow(1);
    outFrustum.planes[ Frustum::Top ]    = m.getRow(3) - m.getRow(1);
    outFrustum.planes[ Frustum::Near ]   = m.getRow(3) + m.getRow(2);
    outFrustum.planes[ Frustum::Far ]    = m.getRow(3) - m.getRow(2);
  }

  bool outsideFrustum (const Frustum &frustum, Vector3 box[8])
  {
    //Test against frustum
    for (int p=0; p<6; ++p) {

      int outCount = 0;

      //Test how many points is outside
      for (int b=0; b<8; ++b)
        if (frustum.testPoint( box[ b ], p ) == Frustum::Outside)
          outCount++;

      //The box is out if all points are outside any one of the planes
      if (outCount == 8)
        return true;
    }

    return false;
  }

  void Renderer::traverseScene (Scene3D *scene, RenderTarget::Enum target)
  {
    //Camera center in world coordinates
    Matrix4x4 camMat = curCamera->getGlobalMatrix();
    Vector3 eye = camMat.getColumn(3).xyz();

    //Frustum for culling
    Frustum frustum;
    if (target == RenderTarget::ShadowMap)
    {
      Matrix4x4 proj = curLight->getProjection();
      Matrix4x4 modelview = curLight->getGlobalMatrix().affineNormalize().affineInverse();
      Matrix4x4 m = proj * modelview;
      getFrustum( m, frustum );
    }
    else
    {
      Matrix4x4 proj = curCamera->getProjection( (Float) viewW, (Float) viewH );
      Matrix4x4 modelview = curCamera->getGlobalMatrix().affineNormalize().affineInverse();
      Matrix4x4 m = proj * modelview;
      getFrustum( m, frustum );
    }

    //Traverse the scene
    for (UintSize t=0; t<scene->getTraversal()->size(); ++t)
    {
      TravNode node = scene->getTraversal()->at( t );
      if (node.event == TravEvent::Begin)
      {
        //Push matrix on the stack
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glMultMatrixf( (GLfloat*) node.actor->getMatrix().m );

        //Skip if disabled for rendering
        if (!node.actor->isRenderable())
          continue;

        //Skip if targeting shadow map and not casting shadows
        if (target == RenderTarget::ShadowMap)
          if (node.actor->getCastShadow() == false)
            continue;

        //Get bounding box
        BoundingBox bbox = node.actor->getBoundingBox();
        Matrix4x4 worldMat = node.actor->getGlobalMatrix();

        //Maximum draw distance
        Float maxDist = node.actor->getMaxDrawDistance();
        if (maxDist >= 0.0f)
        {
          //Check distance of bbox center to camera
          Vector3 center = worldMat * bbox.center;
          Float dist = (center - eye).norm();
          if (dist > maxDist) continue;
        }

        //Transform bbox corners to world space
        Vector3 bboxCorners[8];
        bbox.getCorners( bboxCorners );
        for (Uint c=0; c<8; ++c)
          bboxCorners[ c ] = worldMat * bboxCorners[ c ];

        //Frustum culling
        if (outsideFrustum( frustum, bboxCorners ))
          continue;

        //Render geometry
        node.actor->begin();
        node.actor->render( target );
      }
      else if (node.event == TravEvent::End)
      {
        //Pop matrix off the stack
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();

        //Skip if disabled for rendering
        if (!node.actor->isRenderable())
          continue;

        node.actor->end();
      }
    }
  }
  
  void Renderer::renderWindow (Scene *s, Camera *camera)
  {
    if (camera == NULL)
      return;

    //Set camera current
    curCamera = camera;

    //Setup GL state
    GLProgram::UseFixed();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    
    glEnable( GL_COLOR_MATERIAL );

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    camera->updateProjection( (Float) viewW, (Float) viewH );
    camera->updateView();
    
    //Update widget traversal
    if (s->hasChanged()) s->updateChanges();
    const ArrayList <Actor*> &traversal = s->getTraversal();

    //Render the widgets
    for (UintSize t=0; t<traversal.size(); ++t)
    {
      Widget *widget = SafeCast( Widget, traversal[t] );
      if (widget == NULL) continue;

      Matrix4x4 g = widget->getGlobalMatrix();
      glPushMatrix();
      glMultMatrixf( (GLfloat*) g.m );
      widget->draw();
      glPopMatrix();
    }
  }

}/* namespace GE */
