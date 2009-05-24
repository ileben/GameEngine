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
#include "engine/embedit/shadevert.SpotLight.embedded"
#include "engine/embedit/shadefrag.SpotLight.embedded"

namespace GE
{
  DEFINE_CLASS (Renderer);
  
  Renderer::Renderer()
  {
    camera = NULL;
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

    isDofEnabled = false;
    focusDepth = 50.0f;
    focusRange = 50.0f;
    nearFalloff = 150.0f;
    farFalloff = 150.0f;
  }

  void Renderer::setIsDofEnabled (bool onoff) {
    isDofEnabled = onoff;
  }

  bool Renderer::getIsDofEnabled () {
    return isDofEnabled;
  }

  void Renderer::setDofParams (Float fd, Float fr, Float nf, Float ff ) {
    focusDepth = fd;
    focusRange = fr;
    nearFalloff = nf;
    farFalloff = ff;
  }

  void Renderer::setDofParams (const Vector4 &params) {
    setDofParams( params.x, params.y, params.z, params.w );
  }

  Vector4 Renderer::getDofParams () {
    return Vector4( focusDepth, focusRange, nearFalloff, farFalloff );
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
    winW = width;
    winH = height;
    initBuffers();
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
  -----------------------------------------
  Changes the camera used for rendering
  -----------------------------------------*/

  void Renderer::setCamera (Camera *camera) {
    this->camera = camera;
  }

  Camera* Renderer::getCamera() {
    return camera;
  }

  /*
  --------------------------------------
  Rendering interface
  --------------------------------------*/

  Shader* Renderer::findShaderByKey (const ShaderKey &key)
  {
    for (UintSize s=0; s<shaders.size(); ++s)
      if (shaders[s] == key)
        return shaders[s].shader;

    return NULL;
  }

  Shader* Renderer::composeShader (RenderTarget::Enum target,
                                   Actor *geometry,
                                   Material *material)
  {
    ShaderKey key;
    key.target = target;
    
    if (geometry == NULL) key.geomClass = Class(Actor);
    else key.geomClass = geometry->getShaderComposingClass();
    
    if (material == NULL) key.matClass = Class(Material);
    else key.matClass = material->getShaderComposingClass();

    Shader* shader = findShaderByKey( key );
    if (shader != NULL) return shader;

    shader = new Shader;
    if (geometry != NULL) geometry->composeShader( shader );
    if (material != NULL) material->composeShader( shader );

    printf( "\n");
    printf( "========================\n");
    printf( "    Composing shader    \n" );
    printf( "========================\n");
    printf( "Target: %s\n", target == RenderTarget::GBuffer ? "GBuffer" : "ShadowMap" );
    printf( "Geometry: %s\n", key.geomClass->getString() );
    printf( "Material: %s\n", key.matClass->getString() );
    printf( "\n" );

    shader->compose( target );
    
    key.shader = shader;
    shaders.pushBack( key );

    return shader;
  }

  Shader* Renderer::getCurrentShader() {
    return curShader;
  }
  
  Material* Renderer::getCurrentMaterial() {
    return curMaterial;
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
    shaderAmbient = new Shader;
    shaderAmbient->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderAmbient->fromString( Ambient_VertexSource, Ambient_FragmentSource );
    ambientColorSampler = shaderAmbient->getUniformID( "samplerColor" );

    shaderLightSpot = new Shader;
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerSpec" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerParams" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerShadow" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Int, "castShadow" );
    shaderLightSpot->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "winSize" );
    shaderLightSpot->fromString( shadevert_SpotLight_embedded, shadefrag_SpotLight_embedded );

    deferredSampler[ Deferred::Normal ] = shaderLightSpot->getUniformID( "samplerNormal" );
    deferredSampler[ Deferred::Color ] = shaderLightSpot->getUniformID( "samplerColor" );
    deferredSampler[ Deferred::Specular ] = shaderLightSpot->getUniformID( "samplerSpec" );
    deferredSampler[ Deferred::Params ] = shaderLightSpot->getUniformID( "samplerParams" );
    deferredSampler[ Deferred::Shadow ] = shaderLightSpot->getUniformID( "samplerShadow" );
    deferredCastShadow = shaderLightSpot->getUniformID( "castShadow" );
    deferredWinSize = shaderLightSpot->getUniformID( "winSize" );

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
    shaderDofDown->compile( ShaderType::Fragment, DofDown_FS );
    shaderDofDown->link();
    uDofDownColorSampler = shaderDofDown->getUniformID( "samplerColor" );
    uDofDownNormalSampler = shaderDofDown->getUniformID( "samplerNormal" );
    uDofDownPixelSize = shaderDofDown->getUniformID( "pixelSize" );
    uDofDownDofParams = shaderDofDown->getUniformID( "dofParams" );

    shaderDofBlur = new Shader;
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepth" );
    shaderDofBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofBlur->fromString( Dof_VS, DofBlur_FS );
    uDofBlurColorSampler = shaderDofBlur->getUniformID( "samplerColor" );
    uDofBlurPixelSize = shaderDofBlur->getUniformID( "pixelSize" );
    uDofBlurDirection = shaderDofBlur->getUniformID( "direction" );
    uDofBlurRadius = shaderDofBlur->getUniformID( "radius" );
    uDofBlurDepthSampler = shaderDofBlur->getUniformID( "samplerDepth" );
    uDofBlurDofParams = shaderDofBlur->getUniformID( "dofParams" );

    shaderDofNear = new Shader;
    shaderDofNear->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofNear->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDofNear->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofNear->fromString( Dof_VS, DofNear_FS );
    uDofNearColorSampler = shaderDofNear->getUniformID( "samplerColor" );
    uDofNearPixelSize = shaderDofNear->getUniformID( "pixelSize" );
    uDofNearDofParams = shaderDofNear->getUniformID( "dofParams" );

    shaderDepthBlur = new Shader;
    shaderDepthBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDepthBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDepthBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderDepthBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderDepthBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDepthBlur->fromString( Dof_VS, DepthBlur_FS );
    uDepthBlurColorSampler = shaderDepthBlur->getUniformID( "samplerColor" );
    uDepthBlurPixelSize = shaderDepthBlur->getUniformID( "pixelSize" );
    uDepthBlurDirection = shaderDepthBlur->getUniformID( "direction" );
    uDepthBlurRadius = shaderDepthBlur->getUniformID( "radius" );
    uDepthBlurDofParams = shaderDepthBlur->getUniformID( "dofParams" );

    shaderDofMix = new Shader;
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerMedBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerLargeBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepth" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDepthBlur" );
    shaderDofMix->registerUniform( ShaderType::Fragment, DataUnit::Vec4, "dofParams" );
    shaderDofMix->compile( ShaderType::Vertex, Dof_VS );
    shaderDofMix->compile( ShaderType::Fragment, DofMix_FS );
    shaderDofMix->link();
    uDofMixColorSampler = shaderDofMix->getUniformID( "samplerColor" );
    uDofMixMedBlurSampler = shaderDofMix->getUniformID( "samplerMedBlur" );
    uDofMixLargeBlurSampler = shaderDofMix->getUniformID( "samplerLargeBlur" );
    uDofMixDepthSampler = shaderDofMix->getUniformID( "samplerDepth" );
    uDofMixDepthBlurSampler = shaderDofMix->getUniformID( "samplerDepthBlur" );
    uDofMixDofParams = shaderDofMix->getUniformID( "dofParams" );

    shaderBloomDown = new Shader;
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Float, "avgLuminance" );
    shaderBloomDown->registerUniform( ShaderType::Fragment, DataUnit::Float, "maxLuminance" );
    shaderBloomDown->compile( ShaderType::Vertex, Bloom_VS );
    shaderBloomDown->compile( ShaderType::Fragment, BloomDown_FS );
    shaderBloomDown->compile( ShaderType::Fragment, ToneMap_Func );
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
    shaderBloomMix->compile( ShaderType::Fragment, BloomMix_FS );
    shaderBloomMix->compile( ShaderType::Fragment, ToneMap_Func );
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
      glDeleteTextures( 1, &depthDownMap );
      glDeleteTextures( 1, &dofMaxBlurMap );
      glDeleteTextures( 1, &depthMaxBlurMap );
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

    initTexture( &depthMaxBlurMap, GL_RGBA16F, GL_COLOR_ATTACHMENT2, true, blurW, blurH );
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

  void Renderer::renderShadowMap (Light *light, Scene *scene)
  {
    const Uint32 S = 2048;

    if (!shadowInit)
    {
      /*
      glGenTextures( 1, &shadowMap2 );
      glBindTexture( GL_TEXTURE_2D, shadowMap2 );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, S, S, 0, GL_LUMINANCE, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      */
      glGenTextures( 1, &shadowMap );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      
      glGenFramebuffers( 1, &shadowFB );
      glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0 );
      glDrawBuffer( GL_NONE );
      glReadBuffer( GL_NONE );

      shadowInit = true;
    }

    //Setup GL state to render to shadow texture
    glUseProgram( 0 );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 5.0f, 2.0f );
    
    glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
    glClear( GL_DEPTH_BUFFER_BIT );
    
    //Setup view from the lights perspective
    glViewport( 0, 0, S, S );

    Matrix4x4 lightProj = light->getProjection( 1.0f, 1000.0f );
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( (GLfloat*) lightProj.m );
    
    Matrix4x4 lightView = light->getMatrix().affineInverse();
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat*) lightView.m );

    //Render scene
    traverseSceneNoMats( scene );

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

  void Renderer::renderSceneDeferred (Scene *scene)
  {
    //Update scene
    if (scene->hasChanged())
      scene->updateChanges();

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );

    if (camera != NULL) {
      camera->updateProjection( viewW, viewH );
      camera->updateView();
    }

    //Render into geometry textures
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    glClearColor( 0, 0, 0, camera->getFarClipPlane() );
    glClear( GL_COLOR_BUFFER_BIT );

    GLenum clearBuffers[] = {
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
      GL_COLOR_ATTACHMENT4 };
    glDrawBuffers( 3, clearBuffers );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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

    traverseSceneWithMats( scene );

    //Ambient light pass
    shaderAmbient->use();
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

    //Perform shading for each light
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
    {
      Light* light = scene->getLights()->at( l );

      //Render the shadow map if needed
      if (light->getCastShadows())
        renderShadowMap( light, scene );

      //Setup view and projection
      glViewport( viewX, viewY, viewW, viewH );
      
      if (camera != NULL) {
        camera->updateProjection( viewW, viewH );
        camera->updateView();
      }

      //Enable the light
      Matrix4x4 worldCtm = light->getWorldMatrix();
      glMatrixMode( GL_MODELVIEW );
      glMultMatrixf( (GLfloat*) worldCtm.m );
      light->enable( 0 );

      ///////////////////////////////////////////////

      //Clear stencil, only write to stencil buffer
      glUseProgram( 0 );
      glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
      glDrawBuffer( GL_NONE );
      glDepthMask( GL_FALSE );
      glClear( GL_STENCIL_BUFFER_BIT );
      glDisable( GL_BLEND );
      glDisable( GL_LIGHTING );
      glEnable( GL_STENCIL_TEST );
      glEnable( GL_DEPTH_TEST );

      //Incr stencil for pixel in front of light volume back
      glDepthFunc( GL_GEQUAL );
      glStencilFunc( GL_ALWAYS, 0x0, 0xFF );
      glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

      //Render light volume back faces
      glEnable( GL_CULL_FACE );
      glCullFace( GL_FRONT );
      light->renderVolume();

      //Avoid front faces being clipped by the camera near plane
      if (!light->isPointInVolume( camera->getEye(), camera->getNearClipPlane()*2 ))
      {
        //Incr stencil for pixel behind light volume front
        glDepthFunc( GL_LESS );
        glStencilFunc( GL_EQUAL, 0x1, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

        //Render light volume front faces
        glCullFace( GL_BACK );
        light->renderVolume();

        //Allow write only for pixels inside the light volume
        glStencilFunc( GL_EQUAL, 0x2, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
      }
      else
      {
        //Allow write only for pixels in front of light volume back
        glDepthFunc( GL_LESS );
        glStencilFunc( GL_EQUAL, 0x1, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
      }

      //TODO: Check whether any pixels in the volume at all,
      //then render the shadow map and restore the state for
      //rendering to the accumulation framebuffer

      ///////////////////////////////////////////////

      //Render into lighting accumulation texture
      shaderLightSpot->use();
      glUniform2f( deferredWinSize, (GLfloat) winW, (GLfloat) winH );
      glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
      glDrawBuffer( GL_COLOR_ATTACHMENT0 );
      glDisable( GL_DEPTH_TEST );
      glEnable( GL_CULL_FACE );
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE );

      //Bind shadow texture if needed
      if (light->getCastShadows())
      {
        glUniform1i( deferredCastShadow, 1);
        glUniform1i( deferredSampler[Deferred::Shadow], Deferred::Shadow );
        glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
        glBindTexture( GL_TEXTURE_2D, shadowMap );
        glEnable( GL_TEXTURE_2D );
      }
      else glUniform1i( deferredCastShadow, 0);

      //Bind geometry textures
      for (int d=0; d<GE_NUM_GBUFFERS; ++d)
      {
        glUniform1i( deferredSampler[d], d );
        glActiveTexture( GL_TEXTURE0 + d );
        glBindTexture( GL_TEXTURE_2D, deferredMaps[d] );
        glEnable( GL_TEXTURE_2D );
      }

      //Setup camera-eye to light-clip matrix
      Matrix4x4 cam = camera->getMatrix();
      Matrix4x4 lightProj = light->getProjection( 1.0f, 1000.0f );
      Matrix4x4 lightInv = light->getMatrix().affineInverse();
      Matrix4x4 tex = lightProj * lightInv * cam;
      glActiveTexture( GL_TEXTURE0 );
      glMatrixMode( GL_TEXTURE );
      glLoadMatrixf( (GLfloat*) tex.m );
      
      //Render light volume back faces
      glCullFace( GL_FRONT );
      light->renderVolume();
      
      //Restore texture state
      for (int d=GE_NUM_GBUFFERS-1; d>=0; --d)
      {
        glActiveTexture( GL_TEXTURE0 + d );
        glDisable( GL_TEXTURE_2D );
      }

      glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
      glDisable( GL_TEXTURE_2D );

      //Restore other state
      glDisable( GL_STENCIL_TEST );
      glCullFace( GL_BACK );
      glDepthMask( GL_TRUE );
      
      /*
      //Draw light volume
      glUseProgram( 0 );
      glDisable( GL_DEPTH_TEST );
      glEnable( GL_CULL_FACE );
      glDisable( GL_BLEND );
      glDisable( GL_LIGHTING );
      glEnable( GL_COLOR_MATERIAL );
      glColor3f( 0,0,1 );
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      light->renderVolume();

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      */
    }
  }

  void Renderer::doToon (Uint32 sourceTex, Uint32 targetFB, Uint32 targetAtch)
  {
    float focusZ = focusDepth;
    float focusW = focusRange;
    float farW = focusRange + farFalloff;
    float nearW = focusRange + nearFalloff;

    ///////////////////////////////////////////////////
    //Apply toon shading and initialize CoC values.
    //Render target: dofMap

    glBindFramebuffer( GL_FRAMEBUFFER, targetFB );
    if (targetFB != 0) glDrawBuffer( targetAtch );

    shaderDofInit->use();
    glUniform4f( uDofInitDofParams, focusZ, focusW, farW, nearW );

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
    float focusZ = focusDepth;
    float focusW = focusRange;
    float farW = focusRange + farFalloff;
    float nearW = focusRange + nearFalloff;

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
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    
    shaderDofNear->use();
    glUniform2f( uDofNearPixelSize, 1.0/winW, 1.0/winH );
    glUniform4f( uDofNearDofParams, focusZ, focusW, farW, nearW );

    glUniform1i( uDofNearColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();
    
    /////////////////////////////////////////////
    //Downsample color and CoC information.
    //Render target: dofDownMap

    glDrawBuffer( GL_COLOR_ATTACHMENT1 );

    shaderDofDown->use();
    glUniform2f( uDofDownPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uDofDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad();

    //////////////////////////////////////////////////
    //Blur downsampled depth with large blur radius
    //Render target: depthMaxBlurMap
  
    glDrawBuffer( GL_COLOR_ATTACHMENT2 );

    shaderDepthBlur->use();
    glUniform1i( uDepthBlurRadius, maxBlurRadius );
    glUniform2f( uDepthBlurPixelSize, 1.0/blurW, 1.0/blurH);
    glUniform4f( uDepthBlurDofParams, focusZ, focusW, farW, nearW );

    glUniform1i( uDepthBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, depthDownMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    //////////////////////////////////////////////////////////
    //Blur downsampled color and CoC with large blur radius
    //Render target: dofMaxBlurMap
  
    glDrawBuffer( GL_COLOR_ATTACHMENT3 );

    shaderDofBlur->use();
    glUniform1i( uDofBlurRadius, maxBlurRadius );
    glUniform2f( uDofBlurPixelSize, 1.0/blurW, 1.0/blurH);
    glUniform4f( uDofBlurDofParams, focusZ, focusW, farW, nearW );

    glUniform1i( uDofBlurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofDownMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofBlurDepthSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, depthDownMap );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    /////////////////////////////////////////////////
    //Mix blurred and original DoF images
    //Render target: input argument

    glViewport( viewX, viewY, viewW, viewH);
    glBindFramebuffer( GL_FRAMEBUFFER, targetFB );
    if (targetFB != 0) glDrawBuffer( targetAtch );

    shaderDofMix->use();
    glUniform4f( uDofMixDofParams, focusZ, focusW, farW, nearW );

    glUniform1i( uDofMixColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, dofMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixMedBlurSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, dofMedBlurMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixLargeBlurSampler, 2 );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, dofMaxBlurMap );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixDepthSampler, 3 );
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofMixDepthBlurSampler, 4 );
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, depthMaxBlurMap );
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
    glUniform2f( uBloomDownPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uBloomDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTex );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    ///////////////////////////////////////////////////
    //Blur bloomDownMap in X direction to bloomBlurMap
  
    glDrawBuffer( GL_COLOR_ATTACHMENT5 );

    shaderBloomBlur->use();
    glUniform2f( uBloomBlurPixelSize, 1.0/blurW, 1.0/blurH);
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
    
    if (isDofEnabled)
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

  void Renderer::renderScene (Scene *scene)
  {
    //Update scene
    if (scene->hasChanged())
      scene->updateChanges();

    //Render shadow map for the first light
    if (!scene->getLights()->empty())
      renderShadowMap( scene->getLights()->first(), scene );

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    
    if (camera != NULL) {
      camera->updateProjection( viewW, viewH );
      camera->updateView();
    }

    //Setup camera-eye to light-clip matrix
    if (!scene->getLights()->empty())
    {
      Light *l = scene->getLights()->first();
      
      Matrix4x4 cam = camera->getMatrix();
      Matrix4x4 lightProj = l->getProjection( 1.0f, 1000.0f );
      Matrix4x4 lightInv = l->getMatrix().affineInverse();
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
      Matrix4x4 worldCtm = light->getWorldMatrix();
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glMultMatrixf( (GLfloat*) worldCtm.m );
      
      //Setup light data
      light->enable( (int)l );
      
      glPopMatrix();
    }

    //Render geometry with materials
    traverseSceneWithMats( scene );

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

  void Renderer::traverseSceneNoMats (Scene *scene)
  {
    for (UintSize t=0; t<scene->getTraversal()->size(); ++t)
    {
      TravNode node = scene->getTraversal()->at( t );
      switch (node.event)
      {
      case TravEvent::Begin:
        
        node.actor->begin();

        if (!node.actor->isRenderable())
          continue;

        curMaterial = NULL;
        curShader = composeShader( RenderTarget::ShadowMap, node.actor, curMaterial );
        curShader->use();

        node.actor->render( GE_ANY_MATERIAL_ID );

        curShader = NULL;
        glUseProgram(0);

        break;
      case TravEvent::End:

        node.actor->end();
        break;
      }
    }
  }

  void Renderer::traverseSceneWithMats (Scene *scene)
  {
    for (UintSize t=0; t<scene->getTraversal()->size(); ++t)
    {
      TravNode node = scene->getTraversal()->at( t );
      if (node.event == TravEvent::Begin)
      {
        node.actor->begin();

        //Skip if disabled for rendering
        if (!node.actor->isRenderable())
          continue;

        //Find the type of material
        Material *mat = node.actor->getMaterial();
        if (mat == NULL) {
          
          //Compose shader
          curMaterial = NULL;
          curShader = composeShader( RenderTarget::GBuffer, node.actor, curMaterial );
          curShader->use();

          //Use default material if none
          Material::BeginDefault();
          node.actor->render( GE_ANY_MATERIAL_ID );
          
        }else if (ClassOf(mat) == Class(MultiMaterial)) {
          
          //Iterate through the sub materials
          MultiMaterial *mmat = (MultiMaterial*) mat;
          for (UintSize s=0; s<mmat->getNumSubMaterials(); ++s)
          {
            //Compose shader
            curMaterial = mmat->getSubMaterial((MaterialID)s);
            curShader = composeShader( RenderTarget::GBuffer, node.actor, curMaterial );
            curShader->use();

            //Render with each sub-material
            mmat->selectSubMaterial( (MaterialID)s );
            mmat->begin();
            node.actor->render( (MaterialID)s );
            mmat->end();
          }
        
        }else{

          //Compose shader
          curMaterial = mat;
          curShader = composeShader( RenderTarget::GBuffer, node.actor, curMaterial );
          curShader->use();

          //Render with given material
          mat->begin();
          node.actor->render( GE_ANY_MATERIAL_ID );
          mat->end();
        }
      }
      else if (node.event == TravEvent::End)
      {
        node.actor->end();
      }
    }
  }
  
  void Renderer::renderWidget( Widget *w )
  {
    //Setup GL state
    GLProgram::UseFixed();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    
    glEnable( GL_COLOR_MATERIAL );

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    
    if (camera != NULL) {
      camera->updateProjection( viewW, viewH );
      camera->updateView();
    }
    
    //Render the widget
    w->draw();
  }

}/* namespace GE */
