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

  void Renderer::drawActor( Actor *actor )
  {
    actor->renderBegin();

    //Find the type of material
    Material *mat = actor->getMaterial();
    if( mat == NULL ){
      
      //Use default if none
      Material::BeginDefault();
      actor->renderGeometry( GE_ANY_MATERIAL_ID );

    }else if( ClassOf(mat) == Class(MultiMaterial) ){
      
      //Render with each sub-material if multi
      MultiMaterial *mmat = (MultiMaterial*) mat;
      for( UintSize s=0; s<mmat->getNumSubMaterials(); ++s )
      {
        mmat->selectSubMaterial( (MaterialID)s );
        mmat->begin();
        actor->renderGeometry( (MaterialID)s );
        mmat->end();
      }
    
    }else{
      
      //Render with given material
      mat->begin();
      actor->renderGeometry( GE_ANY_MATERIAL_ID );
      mat->end();
    }

    //Recurse for each child
    Group *grp = SafeCast( Group, actor );
    if (grp != NULL) {
      for (UintSize c=0; c<grp->getChildren()->size(); ++c)
        drawActor( grp->getChildren()->at( c ) ); }
    
    actor->renderEnd();
  }
  
  void Renderer::drawWidget( Widget *w )
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
