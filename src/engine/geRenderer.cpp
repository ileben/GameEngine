#include "engine/geRenderer.h"
#include "engine/geCamera.h"
#include "engine/geLight.h"
#include "engine/geShaders.h"
#include "engine/geScene.h"
#include "engine/geShader.h"
#include "widgets/geWidget.h"
#include "engine/geGLHeaders.h"
#include "engine/actors/geSkinMeshActor.h"

#include "engine/embedit/ToneMap.embedded"
#include "engine/embedit/Ambient.embedded"
#include "engine/embedit/Dof.embedded"
#include "engine/embedit/Down.embedded"
#include "engine/embedit/Bloom.embedded"
#include "engine/embedit/Blur.embedded"
#include "engine/embedit/Cell.embedded"
#include "engine/embedit/Final.embedded"
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

    shaderDofInit = new Shader;
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shaderDofInit->registerUniform( ShaderType::Fragment, DataUnit::Vec3, "dofParams" );
    shaderDofInit->fromString( Dof_VertexSource, DofInit_FragmentSource );
    uDofInitColorSampler = shaderDofInit->getUniformID( "samplerColor" );
    uDofInitNormalSampler = shaderDofInit->getUniformID( "samplerNormal" );
    uDofInitDofParams = shaderDofInit->getUniformID( "dofParams" );

    shaderDown = new Shader;
    shaderDown->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderDown->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderDown->fromString( Down_VertexSource, Down_FragmentSource );
    uDownColorSampler = shaderDown->getUniformID( "samplerColor" );
    uDownPixelSize = shaderDown->getUniformID( "pixelSize" );

    shaderBloom = new Shader;
    shaderBloom->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBloom->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderBloom->registerUniform( ShaderType::Fragment, DataUnit::Float, "avgLuminance" );
    shaderBloom->registerUniform( ShaderType::Fragment, DataUnit::Float, "maxLuminance" );
    shaderBloom->compile( ShaderType::Vertex, Bloom_VertexSource );
    shaderBloom->compile( ShaderType::Fragment, Bloom_FragmentSource );
    shaderBloom->compile( ShaderType::Fragment, ToneMapSource );
    shaderBloom->link();
    uBloomColorSampler = shaderBloom->getUniformID( "samplerColor" );
    uBloomPixelSize = shaderBloom->getUniformID( "pixelSize" );
    uBloomAvgLuminance = shaderBloom->getUniformID( "avgLuminance" );
    uBloomMaxLuminance = shaderBloom->getUniformID( "maxLuminance" );

    shaderCell = new Shader;
    shaderCell->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderCell->fromString( Cell_VertexSource, Cell_FragmentSource );
    cellColorSampler = shaderCell->getUniformID( "samplerColor" );

    shaderBlur = new Shader;
    shaderBlur->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderBlur->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "direction" );
    shaderBlur->registerUniform( ShaderType::Fragment, DataUnit::Int, "radius" );
    shaderBlur->fromString( Blur_VertexSource, Blur_FragmentSource );
    blurColorSampler = shaderBlur->getUniformID( "samplerColor" );
    uBlurPixelSize = shaderBlur->getUniformID( "pixelSize" );
    uBlurDirection = shaderBlur->getUniformID( "direction" );
    uBlurRadius = shaderBlur->getUniformID( "radius" );

    shaderFinal = new Shader;
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerBloom" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerDof" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Float, "avgLuminance" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Float, "maxLuminance" );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "poissonCoords", 12 );
    shaderFinal->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "pixelSize" );
    shaderFinal->compile( ShaderType::Vertex, Final_VertexSource );
    shaderFinal->compile( ShaderType::Fragment, Final_FragmentSource );
    shaderFinal->compile( ShaderType::Fragment, ToneMapSource );
    shaderFinal->link();
    uFinalColorSampler = shaderFinal->getUniformID( "samplerColor" );
    uFinalNormalSampler = shaderFinal->getUniformID( "samplerNormal" );
    uFinalBloomSampler = shaderFinal->getUniformID( "samplerBloom" );
    uFinalDofSampler = shaderFinal->getUniformID( "samplerDof" );
    uFinalAvgLuminance = shaderFinal->getUniformID( "avgLuminance" );
    uFinalMaxLuminance = shaderFinal->getUniformID( "maxLuminance" );
    uFinalPoissonCoords = shaderFinal->getUniformID( "poissonCoords" );
    uFinalPixelSize = shaderFinal->getUniformID( "pixelSize" );

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
      glDeleteRenderbuffers( 1, &deferredDepth );
      glDeleteTextures( 1, &deferredAccum );
      glDeleteTextures( GE_NUM_GBUFFERS, deferredMaps );

      glDeleteFramebuffers( 1, &blurFB );
      glDeleteTextures( 1, &blurMaps[0] );
      glDeleteTextures( 1, &blurMaps[1] );
      glDeleteTextures( 1, &effectsMaps[0] );
      glDeleteTextures( 1, &effectsMaps[1] );
    };

    //Generate deferred framebuffer
    glGenFramebuffers( 1, &deferredFB );
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    //Generate deferred depth-stencil buffer
    glGenRenderbuffers( 1, &deferredDepth );
    glBindRenderbuffer( GL_RENDERBUFFER, deferredDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winW, winH );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );

    //Generate deferred geometry buffers
    glGenTextures( GE_NUM_GBUFFERS, deferredMaps );
    initTexture( &deferredMaps[ Deferred::Normal ], GL_RGBA16F, GL_COLOR_ATTACHMENT1 );
    initTexture( &deferredMaps[ Deferred::Color ], GL_RGBA8, GL_COLOR_ATTACHMENT2 );
    initTexture( &deferredMaps[ Deferred::Specular ], GL_RGBA8, GL_COLOR_ATTACHMENT3 );
    initTexture( &deferredMaps[ Deferred::Params ], GL_RGBA16F, GL_COLOR_ATTACHMENT4 );

    //Generate deferred lighting accumulation buffer
    initTexture( &deferredAccum, GL_RGBA16F, GL_COLOR_ATTACHMENT0, true );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Generate deferred dof buffer
    initTexture( &deferredDof, GL_RGBA16F, GL_COLOR_ATTACHMENT5, true );
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

    initTexture( &blurMaps[0], GL_RGBA16F, GL_COLOR_ATTACHMENT0, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    initTexture( &blurMaps[1], GL_RGBA16F, GL_COLOR_ATTACHMENT1, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    initTexture( &effectsMaps[0], GL_RGBA16F, GL_COLOR_ATTACHMENT2, true, blurW, blurH );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    initTexture( &effectsMaps[1], GL_RGBA16F, GL_COLOR_ATTACHMENT3, true, blurW, blurH );
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
    glPolygonOffset( 2.0f, 2.0f );
    
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
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    //GLenum drawBuffers[] = {
    //  GL_COLOR_ATTACHMENT0,
    //  GL_COLOR_ATTACHMENT5 };
    //glDrawBuffers( 2, drawBuffers );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
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
    GLenum drawBuffers[] = {
      GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
      GL_COLOR_ATTACHMENT4 };
    glDrawBuffers( 4, drawBuffers );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

  void doBlur ()
  {
    
  }

  void Renderer::endDeferred()
  {
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glColor3f( 1,1,1 );

    GLfloat poissonCoords[] = {
      -0.326212f, -0.40581f,
      -0.840144f, -0.07358f,
      -0.695914f,  0.457137f,
      -0.203345f,  0.620716f,
       0.96234f,  -0.194983f,
       0.473434f, -0.480026f,
       0.519456f,  0.767022f,
       0.185461f, -0.893124f,
       0.507431f,  0.064425f,
       0.89642f,   0.412458f,
      -0.32194f,  -0.932615f,
      -0.791559f, -0.59771f
    };

    ////////////////////////////////////////
    //Initialize DoF blur values

    glDrawBuffer( GL_COLOR_ATTACHMENT5 );

    shaderDofInit->use();
    glUniform3f( uDofInitDofParams, 10.0f, 40.0f, 100.0f );

    glUniform1i( uDofInitColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredAccum );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uDofInitNormalSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ]);
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad ();
    /*

    ////////////////////////////////////////
    //Downsample to blurMaps[0]

    glViewport( 0,0,blurW,blurH );
    glBindFramebuffer( GL_FRAMEBUFFER, blurFB );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    shaderDown->use();
    glUniform2f( uDownPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uDownColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredAccum );
    glEnable( GL_TEXTURE_2D );

    fullScreenQuad ();

    ////////////////////////////////////////
    //Blur in X direction to blurMaps[1]
  
    glDrawBuffer( GL_COLOR_ATTACHMENT1 );

    shaderBlur->use();
    glUniform2f( uBlurPixelSize, 1.0/blurW, 1.0/blurH);
    glUniform2f( uBlurDirection, 1.0, 0.0 );
    glUniform1i( uBlurRadius, 5 );

    glUniform1i( blurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, blurMaps[0] );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad ();

    ////////////////////////////////////////
    //Blur in Y direction to effectsMaps[1]

    glDrawBuffer( GL_COLOR_ATTACHMENT3 );

    glUniform2f( uBlurDirection, 0.0, 1.0 );

    glUniform1i( blurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, blurMaps[1] );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad ();

    //////////////////////////////////////////
    //Downsample, tone map and bloom cutoff
    //to blurMaps[0]

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    shaderBloom->use();
    glUniform1f( uBloomAvgLuminance, avgLuminance );
    glUniform1f( uBloomMaxLuminance, maxLuminance );
    glUniform2f( uBloomPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uBloomColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredAccum );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad();

    ////////////////////////////////////////
    //Blur in X direction to blurMaps[1]
  
    glDrawBuffer( GL_COLOR_ATTACHMENT1 );

    shaderBlur->use();
    glUniform2f( uBlurPixelSize, 1.0/blurW, 1.0/blurH);
    glUniform2f( uBlurDirection, 1.0, 0.0 );
    glUniform1i( uBlurRadius, 14 );

    glUniform1i( blurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, blurMaps[0] );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad ();

    ////////////////////////////////////////
    //Blur in Y direction to effectsMaps[0]

    glDrawBuffer( GL_COLOR_ATTACHMENT2 );

    glUniform2f( uBlurDirection, 0.0, 1.0 );

    glUniform1i( blurColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, blurMaps[1] );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad ();
*/
    ////////////////////////////////////////
    //Bloom + DoF + original image

    glViewport( viewX, viewY, viewW, viewH);
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    shaderFinal->use();
    glUniform1f( uFinalAvgLuminance, avgLuminance );
    glUniform1f( uFinalMaxLuminance, maxLuminance );
    glUniform2fv( uFinalPoissonCoords, 12, poissonCoords );
    glUniform2f( uFinalPixelSize, 1.0/winW, 1.0/winH );

    glUniform1i( uFinalColorSampler, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, deferredAccum );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uFinalBloomSampler, 1 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, effectsMaps[0] );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uFinalDofSampler, 2 );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, deferredDof );
    glEnable( GL_TEXTURE_2D );

    glUniform1i( uFinalNormalSampler, 3 );
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ Deferred::Normal ] );
    glEnable( GL_TEXTURE_2D );
    
    fullScreenQuad ();

    ////////////////////////////////////////

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
