#include <app/app.h>
#include <engine/geGLHeaders.h>


/*
-------------------------------------------
Animations
-------------------------------------------*/

static AnimateFunc animateFunc = NULL;
static KeyFunc keyDownFunc = NULL;

static Renderer *renderer = NULL;

static Scene *scene2D = NULL;
static Camera2D *cam2D = NULL;
static UICtrl *uiCtrl = NULL;

static Scene3D *scene = NULL;
static Scene3D *sceneRender = NULL;
static Camera3D *camRender = NULL;
static FpsController* camCtrl = NULL;
static ArrayList< FpsController* > camCtrls;

static std::map< CharString, AnimController* > anims;
typedef std::map< CharString, AnimController* >::iterator AnimMapIter;

static LinkedList< AnimController* > animQueue;
typedef LinkedList< AnimController* >::Iterator AnimIter;


void appAnimateFunc (AnimateFunc func)
{
  animateFunc = func;
}

void appKeyDownFunc (KeyFunc func)
{
  keyDownFunc = func;
}

AnimController* appPlayAnim (const CharString &name)
{
  AnimController *ctrl = appGetAnim( name );
  if (ctrl == NULL) return NULL;
  animQueue.pushBack( ctrl );
  ctrl->play();
  return ctrl;
}

AnimController* appGetAnim (const CharString &name)
{
  AnimMapIter i = anims.find( name );
  if (i == anims.end()) return NULL;
  return i->second;
}

static void animate()
{
  //Tick kernel
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );

  //Tick animations
  for (AnimIter i=animQueue.begin(); i!=animQueue.end(); )
  {
    AnimController *anim = *i;
    anim->tick();

    if (!anim->isPlaying())
      i = animQueue.removeAt( i );
    else ++i;
  }

  //Tick camera controller
  for (UintSize c=0; c<camCtrls.size(); ++c)
    camCtrls[ c ]->tick();

  //Invoke callback
  if (animateFunc != NULL)
    animateFunc();

  glutPostRedisplay();
}

static void mouseClick (int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON)
    uiCtrl->mouseClick( button, state, x, y );
  else
    camCtrl->mouseClick( button, state, x, y );
}

static void mouseMove (int x, int y)
{
  uiCtrl->mouseMove( x, y );
  camCtrl->mouseMove( x, y );
}

static void keyDown (unsigned char key, int x, int y)
{
  camCtrl->keyDown( key );

  switch (key)
  {
  case 9://tab
    //renderer->setIsDofEnabled( !renderer->getIsDofEnabled() );
    for (UintSize l=0; l<scene->getLights()->size(); ++l)
      scene->getLights()->at( l )->setCastShadows( !scene->getLights()->at( l )->getCastShadows() );
    break;

  case 8://backspace
    break;

  case 13://return
    break;

  case 27:
    //Quit on escape
    exit(0);
  }

  //Invoke callback
  if (keyDownFunc != NULL)
    keyDownFunc( key );
}

static void specialKey (int key, int x, int y)
{
  float l;
  float lstep = 0.05f;
  float fstep = 5.0f;
  Vector4 dof;

  switch (key)
  {
  case GLUT_KEY_F1:
    l = renderer->getAvgLuminance();
    l = Util::Max( l-lstep, 0.0f );
    renderer->setAvgLuminance( l );
    std::cout << "Luminance: " << l << std::endl;
    break;
  case GLUT_KEY_F2:
    l = renderer->getAvgLuminance();
    l = l+lstep;
    renderer->setAvgLuminance( l );
    std::cout << "Luminance: " << l << std::endl;
    break;
  case GLUT_KEY_F4:
    dof = renderer->getDofParams();
    dof.x = Util::Max( dof.x-fstep, 0.0f );
    renderer->setDofParams( dof );
    std::cout << "Focus distance: " << dof.x << std::endl;
    break;
  case GLUT_KEY_F5:
    dof = renderer->getDofParams();
    dof.x = dof.x + fstep;
    renderer->setDofParams( dof );
    std::cout << "Focus distance: " << dof.x << std::endl;
    break;
  }
}

static void keyUp (unsigned char key, int x, int y)
{
  camCtrl->keyUp( key );
}

static void reshape (int w, int h)
{
  if (renderer != NULL)
    renderer->setWindowSize( w,h );
}

static void display ()
{
  Vector2 winSize = renderer->getWindowSize();
  renderer->setViewport( 0,0, (int)winSize.x, (int)winSize.y );

  //switch camera
  renderer->beginFrame();
  
  //3D scene
  renderer->setCamera( camRender );
  renderer->beginDeferred();
  renderer->renderSceneDeferred( sceneRender );
  renderer->endDeferred();
  
  //2D scene
  renderer->setCamera( cam2D );
  renderer->renderWindow( scene2D );
  
  renderer->endFrame();
}

void appInit (int resX, int resY)
{
  int argc = 0;

  //Init GLUT
  glutInit( &argc, NULL );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Game App" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyDown );
  glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  glutMouseFunc( mouseClick );
  glutMotionFunc( mouseMove );
  glutPassiveMotionFunc( mouseMove );
  glutIdleFunc( animate );

  //Create kernel
  Kernel *kernel = new Kernel;
  kernel->enableVerticalSync( false );
  renderer = kernel->getRenderer();
  renderer->setWindowSize( resX, resY );
}

Scene3D* appScene3D (const CharString &filename)
{
  //Load scene file
  scene = Kernel::GetInstance()->loadSceneFile( filename );

  //Map all the animations by name
  for (UintSize a=0; a<scene->animations.size(); ++a)
  {
    //Create a controller for animation
    Animation *anim = scene->animations[ a ];
    AnimController *ctrl = new AnimController;
    ctrl->bindAnimation( anim );

    //Map by name
    anims[ anim->name ] = ctrl;
  }
  
  //Switch to last scene implicitly
  sceneRender = scene;
  return scene;
}

Scene* appScene2D ()
{
  //Create scene
  Stage *stage = new Stage;
  scene2D = new Scene;
  scene2D->setRoot( new Actor );

  //Assign UI controller
  uiCtrl = new UICtrl;
  uiCtrl->bindScene( scene2D );

  //Create UI camera
  cam2D = new Camera2D;

  return scene2D;
}

void appRun ()
{
  //Tick first time after all setup done
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );

  //Run application
  glutMainLoop();
}

FpsController* appCamCtrl (Camera3D *cam)
{
  //Create camera controller
  camCtrl = new FpsController;
  camCtrl->attachCamera( cam );
  camCtrl->setMoveSpeed( 400 );
  camCtrls.pushBack( camCtrl );
  
  //Switch to last camera implicitly
  camRender = cam;
  return camCtrl;
}

void appSwitchCamera (Camera3D *cam)
{
  camRender = cam;
}

void appSwitchScene (Scene3D *scene)
{
  sceneRender = scene;
}
