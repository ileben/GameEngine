#include "engine/geRenderer.h"
#include "engine/geCamera.h"
#include "engine/geLight.h"
#include "engine/geShaders.h"
#include "engine/geScene.h"
#include "engine/geShader.h"
#include "widgets/geWidget.h"
#include "engine/geGLHeaders.h"

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
    deferredInit = false;
  }

  void Renderer::setBackColor (const Vector3 &color)
  {
    back = color;
  }

  void Renderer::setWindowSize (int width, int height)
  {
    winW = width;
    winH = height;
    updateBuffers();
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

  void Renderer::updateBuffers ()
  {
    //Destroy old deferred buffers if present
    if (deferredInit)
    {
      glDeleteTextures( 4, deferredMaps );
      glDeleteRenderbuffers( 1, &deferredDepth );
      glDeleteFramebuffers( 1, &deferredFB );
    };

    //Generate new deferred buffers
    glGenFramebuffers( 1, &deferredFB );
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );

    glGenRenderbuffers( 1, &deferredDepth );
    glBindRenderbuffer( GL_RENDERBUFFER, deferredDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, winW, winH );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );

    GLenum drawBuffers[4];
    glGenTextures( 4, deferredMaps );
    for (Uint32 d=0; d<4; ++d)
    {
      glBindTexture( GL_TEXTURE_2D, deferredMaps[ d ] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, winW, winH, 0, GL_RGBA, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + d, GL_TEXTURE_2D, deferredMaps[d], 0 );
      drawBuffers[d] = GL_COLOR_ATTACHMENT0 + d;
    }
    
    glDrawBuffers( 4, drawBuffers );
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if (status != GL_FRAMEBUFFER_COMPLETE)
      printf( "Deferred framebuffer INCOMPLETE! (status: 0x%x)\n", (int)status );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    //Load deferred light shader once
    if (!deferredInit)
    {
      deferredShader = new Shader;
      deferredShader->fromFile( "deferred_light.vert.c", "deferred_light.frag.c" );
      const GLProgram *deferredProgram = deferredShader->getGLProgram();
      deferredSampler[ Deferred::Vertex ] = deferredProgram->getUniform( "samplerVertex" );
      deferredSampler[ Deferred::Normal ] = deferredProgram->getUniform( "samplerNormal" );
      deferredSampler[ Deferred::Color ] = deferredProgram->getUniform( "samplerColor" );
      deferredSampler[ Deferred::Specular ] = deferredProgram->getUniform( "samplerSpec" );
      deferredSampler[ Deferred::Shadow ] = deferredProgram->getUniform( "samplerShadow" );
      glUseProgram( 0 );
    }

    deferredInit = true;
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
      //glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
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
    
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 2.0f, 2.0f );
    glCullFace( GL_FRONT );
    
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
    glCullFace( GL_BACK );
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

    //Render deferred textures
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( viewX, viewY, viewW, viewH );

    traverseSceneWithMats( scene );

    //Perform shading for each light
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
    {
      Light* light = scene->getLights()->at( l );

      //Render the shadow map
      renderShadowMap( light, scene );

      //Setup view and projection
      glViewport( viewX, viewY, viewW, viewH );
      
      if (camera != NULL) {
        camera->updateProjection( viewW, viewH );
        camera->updateView();
      }

      //Setup camera-eye to light-clip matrix
      Matrix4x4 cam = camera->getMatrix();
      Matrix4x4 lightProj = light->getProjection( 1.0f, 1000.0f );
      Matrix4x4 lightInv = light->getMatrix().affineInverse();
      Matrix4x4 tex = lightProj * lightInv * cam;
      
      glMatrixMode( GL_TEXTURE );
      glLoadMatrixf( (GLfloat*) tex.m );

      //Enable the light
      Matrix4x4 worldCtm = light->getWorldMatrix();
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glMultMatrixf( (GLfloat*) worldCtm.m );
      light->enable( 0 );
      glPopMatrix();

      //Setup the deferred shader and textures
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
      glDisable( GL_DEPTH_TEST );
      glDisable( GL_CULL_FACE );
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE );      
      deferredShader->use();

      glUniform1i( deferredSampler[Deferred::Shadow], Deferred::Shadow );
      glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
      glEnable( GL_TEXTURE_2D );

      for (int d=0; d<4; ++d)
      {
        glUniform1i( deferredSampler[d], d );
        glActiveTexture( GL_TEXTURE0 + d );
        glBindTexture( GL_TEXTURE_2D, deferredMaps[d] );
        glEnable( GL_TEXTURE_2D );
      }
      
      //Shader does no transformation (only viewport applied)
      glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f, -1.0f );
      glTexCoord2f( 1.0f, 0.0f ); glVertex2f( +1.0f, -1.0f );
      glTexCoord2f( 1.0f, 1.0f ); glVertex2f( +1.0f, +1.0f );
      glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f, +1.0f );
      glEnd();

      //Disable textures
      glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
      glDisable( GL_TEXTURE_2D );

      for (int d=3; d>=0; --d)
      {
        glActiveTexture( GL_TEXTURE0 + d );
        glDisable( GL_TEXTURE_2D );
      }
    }
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

        if (node.actor->isRenderable())
          node.actor->render( GE_ANY_MATERIAL_ID );

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
          
          //Use default if none
          Material::BeginDefault();
          node.actor->render( GE_ANY_MATERIAL_ID );
          
        }else if (ClassOf(mat) == Class(MultiMaterial)) {
          
          //Render with each sub-material if multi
          MultiMaterial *mmat = (MultiMaterial*) mat;
          for (UintSize s=0; s<mmat->getNumSubMaterials(); ++s)
          {
            mmat->selectSubMaterial( (MaterialID)s );
            mmat->begin();
            node.actor->render( (MaterialID)s );
            mmat->end();
          }
        
        }else{
          
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
