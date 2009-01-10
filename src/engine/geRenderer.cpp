#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

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
  }

  void Renderer::setBackColor(const Vector3 &color)
  {
    back = color;
  }

  /*
  ----------------------------------------------------
  Changes the target rendering area on the GL window
  ----------------------------------------------------*/

  void Renderer::setViewport(int x, int y, int width, int height)
  {
    viewX = x;
    viewY = y;
    viewW = width;
    viewH = height;
    glViewport(x, y, width, height);

    if (camera != NULL)
      camera->updateProjection(viewW, viewH);
  }

  /*
  -----------------------------------------
  Changes the camera used for rendering
  -----------------------------------------*/

  void Renderer::setCamera(Camera *camera)
  {
    this->camera = camera;
    camera->updateProjection(viewW, viewH);
    camera->updateView();
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
    
    //Initialize view to current camera
    if (camera != NULL) camera->updateView ();
  }

  void Renderer::beginScene( Actor *root )
  {
    //Init scene data
    sceneRoot = root;
    sceneLights.clear();

    //Walk the scene tree and prepare actors
    ArrayList<Actor*> actorStack;
    actorStack.pushBack( root );
    while (!actorStack.empty())
    {
      //Take last actor off the stack
      Actor* act = actorStack.last();
      actorStack.popBack();
      
      //Prepare the actor for rendering
      act->prepare();
      
      //Store lights
      if (act->getRenderRole() == RenderRole::Light)
        sceneLights.pushBack( (Light*) act );
      
      //Put children actors onto the stack
      for (UintSize c=0; c<act->getChildren()->size(); ++c)
        actorStack.pushBack( act->getChildren()->at(c) );
    }
    
    //Enable lights
    for (UintSize l=0; l<sceneLights.size(); ++l)
    {
      //Setup transformation matrix
      Matrix4x4 worldCtm = sceneLights[l]->getWorldMatrix();
      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glMultMatrixf( (GLfloat*) worldCtm.m );
      
      //Setup light data
      sceneLights[l]->enable( (int) l );
      
      glPopMatrix();
    }
  }

  void Renderer::renderScene()
  {
    renderActor( sceneRoot );
  }

  void Renderer::endScene()
  {
    //Disable lights
    for (UintSize l=0; l<sceneLights.size(); ++l)
      glDisable( GL_LIGHT0 + (int) l );
  }

  void Renderer::endFrame()
  {
    glutSwapBuffers ();
  }

  /*
  ------------------------------------------------------
  Renderer class has authority over the rendering steps
  invoked and their order. This is to allow for special
  rendering passes - e.g. when a shadow map is being
  rendered using materials (and possibly enabling other
  shading programs) might not be desired.
  ------------------------------------------------------*/

  void Renderer::renderActor( Actor *actor )
  {
    //Must be enabled for rendering
    if (!actor->isRenderable()) return;
    
    actor->begin();
    
    //Find the type of material
    Material *mat = actor->getMaterial();
    if( mat == NULL ){
      
      //Use default if none
      Material::BeginDefault();
      actor->render( GE_ANY_MATERIAL_ID );
      
    }else if( ClassOf(mat) == Class(MultiMaterial) ){
      
      //Render with each sub-material if multi
      MultiMaterial *mmat = (MultiMaterial*) mat;
      for( UintSize s=0; s<mmat->getNumSubMaterials(); ++s )
      {
        mmat->selectSubMaterial( (MaterialID)s );
        mmat->begin();
        actor->render( (MaterialID)s );
        mmat->end();
      }
    
    }else{
      
      //Render with given material
      mat->begin();
      actor->render( GE_ANY_MATERIAL_ID );
      mat->end();
    }
    
    //Recurse to each child
    for (UintSize c=0; c<actor->getChildren()->size(); ++c)
      renderActor( actor->getChildren()->at( c ) );
    
    actor->end();
  }
  
  void Renderer::renderWidget( Widget *w )
  {
    GLProgram::UseFixed();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    
    glEnable( GL_COLOR_MATERIAL );
    
    w->draw();
  }

}/* namespace GE */
