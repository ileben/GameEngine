#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;;

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

  /*----------------------------------------------------
   * Changes the target rendering area on the GL window
   *----------------------------------------------------*/

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

  /*-----------------------------------------
   * Changes the camera used for rendering
   *-----------------------------------------*/

  void Renderer::setCamera(Camera *camera)
  {
    this->camera = camera;
    camera->updateProjection(viewW, viewH);
    camera->updateView();
  }

  Camera* Renderer::getCamera() {
    return camera;
  }

  /*--------------------------------------
   * Initializes next frame for rendering
   *--------------------------------------*/

  void Renderer::begin()
  {
    glClearColor (back.x, back.y, back.z, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (camera != NULL) camera->updateView ();
  }

  void Renderer::end()
  {
    glutSwapBuffers ();
  }

  void Renderer::drawActor(Actor *actor)
  {
    Material *mat = actor->getMaterial ();
    if (mat == NULL) {
      
      Material::BeginDefault ();
      actor->render (-1);

    }else if (ClassOf (mat) == Class (MultiMaterial)) {
      
      MultiMaterial *mmat = (MultiMaterial*) mat;
      for (int s=0; s<mmat->getNumSubMaterials(); ++s)
      {
        mmat->selectSubMaterial (s);
        mmat->begin ();
        actor->render (s);
        mmat->end ();
      }

    }else{
      
      mat->begin ();      
      actor->render (-1);
      mat->end ();
    }
  }
  
  void Renderer::drawWidget(Widget *w)
  {
    GLProgram::UseFixed ();
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_CULL_FACE);
    glDisable (GL_LIGHTING);
    glDisable (GL_BLEND);
    
    glEnable (GL_COLOR_MATERIAL);
    
    w->draw ();
  }

}/* namespace GE */
