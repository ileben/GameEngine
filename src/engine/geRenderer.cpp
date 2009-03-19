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
    glBindTexture( GL_TEXTURE_2D, shadowMap );
    glEnable( GL_TEXTURE_2D );
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
    glTexCoord2f( 1, 0 ); glVertex2f( 100, 0 );
    glTexCoord2f( 1, 1 ); glVertex2f( 100, 100 );
    glTexCoord2f( 0, 1 ); glVertex2f( 0, 100 );
    glEnd();
    
    glDisable( GL_TEXTURE_2D );
  }

  void Renderer::renderShadowMap (Light *light, Scene *scene)
  {
    const Uint32 S = 2048;

    if (!shadowInit)
    {
      glGenTextures( 1, &shadowMap2 );
      glBindTexture( GL_TEXTURE_2D, shadowMap2 );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, S, S, 0, GL_LUMINANCE, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      
      glGenTextures( 1, &shadowMap );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
      //glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S, S, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      
      glGenFramebuffers( 1, &shadowFB );
      glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0 );
      
      shadowInit = true;
    }

    //Setup GL state to render to shadow texture
    glUseProgram( 0 );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );

    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 5.0f, 2.0f );
    glCullFace( GL_FRONT );

    glBindFramebuffer( GL_FRAMEBUFFER, shadowFB );
    glDrawBuffer( GL_NONE );
    glReadBuffer( GL_NONE );

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

    //Restore state
    glCullFace( GL_BACK );
    glDisable( GL_POLYGON_OFFSET_FILL );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );


    ////////////////////////////////////////////////////////
    /*
    GLfloat *pixels = new GLfloat[ 512 * 512 ];

    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glBindTexture( GL_TEXTURE_2D, shadowMap );
    glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels );

    
    float lastp = -1.0f;
    for (int y=0; y<512; ++y)
    {
      for (int x=0; x<512; ++x)
      {
        float p = pixels[ y * 512 + x ];
        //if (lastp > 0.0f && p != lastp)
          //printf( "p: %f\n", p );
        //lastp = p;
        float pp = (p - 0.9f) / 0.1f;
        pixels[ y * 512 + x ] = pp;
        //pixels[ y * 512 + x ] = 0.2f;
      }
    }
    
    glBindTexture( GL_TEXTURE_2D, shadowMap2 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_LUMINANCE, GL_FLOAT, pixels );

    delete[] pixels;*/
  }

  void Renderer::beginFrame()
  {
    //Clear the framebuffer
    glClearColor (back.x, back.y, back.z, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Initialize view to current camera
    if (camera != NULL) camera->updateView ();
  }

  void Renderer::beginScene( Scene *scene )
  {
    //Store scene pointer
    this->scene = scene;

    //Update scene
    if (scene->hasChanged())
      scene->updateChanges();

    //Setup view and projection
    glViewport( viewX, viewY, viewW, viewH );
    
    if (camera != NULL) {
      camera->updateProjection( viewW, viewH );
      camera->updateView();
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

    //Setup camera-eye to light-clip matrix
    if (!scene->getLights()->empty())
    {
      Light *l = scene->getLights()->first();

      Matrix4x4 bias;
      bias.set
        (0.5, 0.0, 0.0, 0.5,
         0.0, -0.5, 0.0, 0.5,
         0.0, 0.0, 0.5, 0.5,
         0.0, 0.0, 0.0, 1);
      
      Matrix4x4 cam = camera->getMatrix();
      Matrix4x4 lightProj = l->getProjection( 1.0f, 1000.0f );
      Matrix4x4 lightInv = l->getMatrix().affineInverse();
      Matrix4x4 tex = lightProj * lightInv * cam;
      
      glMatrixMode( GL_TEXTURE );
      glLoadMatrixf( (GLfloat*) tex.m );

      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
    }

    //Prepare the actors for rendering
    for (UintSize t=0; t<scene->getTraversal()->size(); ++t)
    {
      TravNode node = scene->getTraversal()->at( t );
      if (node.event != TravEvent::Begin) continue;
      node.actor->prepare();
    }
  }

  void Renderer::renderSceneDeferred()
  {
    if (!deferredInit)
    {
      glGenTextures( 4, deferredMaps );

      Uint32 d;
      for (d=0; d<4; ++d)
      {
        glBindTexture( GL_TEXTURE_2D, deferredMaps[ d ] );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, viewW, viewH, 0, GL_RGBA, GL_FLOAT, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      }
      
      glGenRenderbuffers( 1, &deferredDepth);
      glBindRenderbuffer( GL_RENDERBUFFER, deferredDepth );
      glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewW, viewH );

      glGenFramebuffers( 1, &deferredFB );
      glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
      glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredDepth );

      GLenum drawBuffers[4];
      for (d=0; d<4; ++d)
      {
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + d, GL_TEXTURE_2D, deferredMaps[d], 0 );
        drawBuffers[d] = GL_COLOR_ATTACHMENT0 + d;
      }
      
      glDrawBuffers( 4, drawBuffers );
      GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

      deferredShader = new Shader;
      deferredShader->fromFile( "deferred_light.vert.c", "deferred_light.frag.c" );
      const GLProgram *deferredProgram = deferredShader->getGLProgram();
      deferredSampler[ Deferred::Vertex ] = deferredProgram->getUniform( "samplerVertex" );
      deferredSampler[ Deferred::Normal ] = deferredProgram->getUniform( "samplerNormal" );
      deferredSampler[ Deferred::Color ] = deferredProgram->getUniform( "samplerColor" );
      deferredSampler[ Deferred::Specular ] = deferredProgram->getUniform( "samplerSpec" );
      glUseProgram( 0 );
      
      deferredInit = true;
    }

    //Render deferred textures
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFB );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, viewW, viewH );

    renderScene();

    //Perform shading for each light

    /*
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glViewport( viewX, viewY, viewW, viewH );
    glClear( GL_COLOR_BUFFER_BIT );

    glUseProgram( 0 );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_BLEND );
    glDisable( GL_LIGHTING );

    glColor3f( 1,1,1 );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, deferredMaps[ 0 ] );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -200, -200 );
    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( +200, -200 );
    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( +200, +200 );
    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -200, +200 );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    */

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glViewport( viewX, viewY, viewW, viewH );
    glClear( GL_COLOR_BUFFER_BIT );
    
    deferredShader->use();
    for (int d=0; d<4; ++d)
    {
      glUniform1i( deferredSampler[d], d );
      glActiveTexture( GL_TEXTURE0 + d );
      glBindTexture( GL_TEXTURE_2D, deferredMaps[d] );
      glEnable( GL_TEXTURE_2D );
    }

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    for (UintSize l=0; l<scene->getLights()->size(); ++l)
    {
      Light* light = scene->getLights()->at( l );
      Matrix4x4 worldCtm = light->getWorldMatrix();
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glMultMatrixf( (GLfloat*) worldCtm.m );
      light->enable( 0 );
      glPopMatrix();

      glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f, -1.0f );
      glTexCoord2f( 1.0f, 0.0f ); glVertex2f( +1.0f, -1.0f );
      glTexCoord2f( 1.0f, 1.0f ); glVertex2f( +1.0f, +1.0f );
      glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f, +1.0f );
      glEnd();
    }

    for (int d=3; d>=0; --d)
    {
      glActiveTexture( GL_TEXTURE0 + d );
      glDisable( GL_TEXTURE_2D );
    }
    glDisable( GL_BLEND );
  }
  
  /*
  ------------------------------------------------------
  Renderer class has authority over the rendering steps
  invoked and their order. This is to allow for special
  rendering passes - e.g. when a shadow map is being
  rendered using materials (and possibly enabling other
  shading programs) might not be desired.
  ------------------------------------------------------*/

  void Renderer::renderScene()
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

  void Renderer::endScene()
  {
    //Disable lights
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
      glDisable( GL_LIGHT0 + (int) l );
  }

  void Renderer::endFrame()
  {
    glutSwapBuffers ();
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
