#include <app/app.h>
#include <engine/geGLHeaders.h>

/*
-------------------------------------------
Crosshair
-------------------------------------------*/

class Crosshair : public Widget
{
  virtual void draw ()
  {
    float S = 5.0f;

    glColor3f( 1,1,1 );
    glBegin( GL_LINES );
    glVertex2f( loc.x - S, loc.y );
    glVertex2f( loc.x + S, loc.y );
    glVertex2f( loc.x, loc.y - S );
    glVertex2f( loc.x, loc.y + S );
    glEnd();
  }
};


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

void appStopAnim (const CharString &name)
{
  AnimController *ctrl = appGetAnim( name );
  AnimIter it = animQueue.iteratorOf( ctrl );
  if (it == animQueue.end()) return;
  
  ctrl->pause();
  animQueue.removeAt( it );
}

void appFinishAnim (const CharString &name)
{
  AnimController *ctrl = appGetAnim( name );
  AnimIter it = animQueue.iteratorOf( ctrl );
  if (it == animQueue.end()) return;

  ctrl->pause();
  ctrl->observeAt( ctrl->getDuration() );
  animQueue.removeAt( it );
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

/*
------------------------------------------------------------
Actor mouse events
------------------------------------------------------------*/


struct ActorCallback
{
  ActorMouseEvent::Enum evt;
  Actor3D *actor;
  ActorFunc func;
  bool on;

  ActorCallback() : on (false) {}
};

ArrayList< ActorCallback > actorCallbacks;

int findActorMouseFunc (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  for (UintSize c=0; c<actorCallbacks.size(); ++c) {
    if (actorCallbacks[ c ].actor == actor &&
        actorCallbacks[ c ].evt == evt) {
     return (int) c;
  }}

  return -1;
}

void appActorMouseFunc (ActorMouseEvent::Enum evt, Actor3D *actor, ActorFunc func)
{
  int existing = findActorMouseFunc( evt, actor );
  if (existing >= 0)
  {
    actorCallbacks[ existing ].func = func;
    return;
  }

  ActorCallback c;
  c.actor = actor;
  c.evt = evt;
  c.func = func;
  actorCallbacks.pushBack( c );
}

void removeActorMouseFunc (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  int existing = findActorMouseFunc( evt, actor );
  if (existing >= 0) actorCallbacks.removeAt( existing );
}

bool actorHitTest (Actor3D *actor, Float x, Float y)
{
  Vector2 winSize = renderer->getWindowSize();
  BoundingBox bbox = appProjectActor( actor, camRender, 0.0f, 0.0f, winSize.x, winSize.y );
  if (bbox.max.z < 0.0f) return false;

  bbox.min.y = winSize.y - bbox.min.y;
  bbox.max.y = winSize.y - bbox.max.y;
  
  Float miny = bbox.min.y;
  bbox.min.y = bbox.max.y;
  bbox.max.y = miny;

  if (x < bbox.min.x || x > bbox.max.x) return false;
  if (y < bbox.min.y || y > bbox.max.y) return false;

  return true;
}

void actorMouseMove (Float x, Float y)
{
  for (UintSize c=0; c < actorCallbacks.size(); ++c)
  {
    ActorCallback &cb = actorCallbacks[ c ];
    if (cb.evt == ActorMouseEvent::Enter)
    {
      bool hit = actorHitTest( cb.actor, x, y );
      if ((!cb.on) && hit)
      {
        cb.func( ActorMouseEvent::Enter, cb.actor );
        cb.on = true;
      }
      else if (cb.on && (!hit))
      {
        cb.func( ActorMouseEvent::Leave, cb.actor );
        cb.on = false;
      }
    }
  }
}

void actorMouseClick (Float x, Float y)
{
  for (UintSize c=0; c < actorCallbacks.size(); ++c)
  {
    ActorCallback &cb = actorCallbacks[ c ];
    if (cb.evt == ActorMouseEvent::Click)
    {
      if (actorHitTest( cb.actor, x, y ))
        cb.func( cb.evt, cb.actor );
    }
  }
}

/*
------------------------------------------------------------
GLUT events
------------------------------------------------------------*/

static void mouseClick (int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {

    //Send left clicks to UI
    uiCtrl->mouseClick( button, state, x, y );
    actorMouseClick( (Float) x, (Float) y );
  }else{

    //Send right clicks to camera
    if (camCtrl->getAttachedCamera() == camRender)
      camCtrl->mouseClick( button, state, x, y );
  }

  //if (state == GLUT_DOWN) {
  //  Vector2 winSize = renderer->getWindowSize();
  //  actorMouseClick( winSize.x * 0.5f, winSize.y * 0.5f );
  //}
}

static void mouseMove (int x, int y)
{
  uiCtrl->mouseMove( x, y );
  actorMouseMove( (Float) x, (Float) y );

  if (camCtrl->getAttachedCamera() == camRender)
    camCtrl->mouseMove( x, y );

  //Vector2 winSize = renderer->getWindowSize();
  //actorMouseMove( winSize.x * 0.5f, winSize.y * 0.5f );
}

static void keyDown (unsigned char key, int x, int y)
{
  if (camCtrl->getAttachedCamera() == camRender)
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
  DofParams dof;

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
    dof = camRender->getDofParams();
    dof.focusCenter = Util::Max( dof.focusCenter - fstep, 0.0f );
    camRender->setDofParams( dof );
    std::cout << "Focus distance: " << dof.focusCenter << std::endl;
    break;
  case GLUT_KEY_F5:
    dof = camRender->getDofParams();
    dof.focusCenter = dof.focusCenter + fstep;
    camRender->setDofParams( dof );
    std::cout << "Focus distance: " << dof.focusCenter << std::endl;
    break;
  }
}

static void keyUp (unsigned char key, int x, int y)
{
  if (camCtrl->getAttachedCamera() == camRender)
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
  renderer->beginDeferred();
  renderer->renderSceneDeferred( sceneRender, camRender );
  renderer->endDeferred();
  
  //2D scene
  renderer->renderWindow( scene2D, cam2D );
  
  renderer->endFrame();
}

/*
-------------------------------------------------
Init functions
-------------------------------------------------*/

void appInit (int resX, int resY)
{
  int argc = 0;

  //Init GLUT
  glutInit( &argc, NULL );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Game App" );

  //glutGameModeString( "1280x1024:32@60" );
  //glutEnterGameMode();
  
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

  /*
  Vector2 winSize = renderer->getWindowSize();
  Crosshair *crosshair = new Crosshair;
  scene2D->getRoot()->addChild( crosshair );
  crosshair->setLoc( winSize.x * 0.5f, winSize.y * 0.5f );
  */

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
  camCtrl->resetState();
}

void appSwitchScene (Scene3D *scene)
{
  sceneRender = scene;
}

BoundingBox appProjectActor (Actor3D *actor, Camera *camera,
                             Float viewX, Float viewY, Float viewW, Float viewH)
{
  //Get the transformation matrix
  Matrix4x4 world = actor->getGlobalMatrix();
  Matrix4x4 modelview = camera->getGlobalMatrix().affineNormalize().affineInverse();
  Matrix4x4 projection = camera->getProjection( viewW, viewH );
  Matrix4x4 m = projection * modelview * world;

  //Get corners of the bounding box
  BoundingBox bboxActor = actor->getBoundingBox();
  Vector3 bboxCorners[8];
  bboxActor.getCorners( bboxCorners );

  //Project each corner into viewport
  BoundingBox bboxOut;
  for (Uint c=0; c<8; ++c)
  {
    //Projection and perspective division
    Vector4 p = m.transformPoint( bboxCorners[ c ].xyz(1.0f) );
    p /= p.w;

    //Apply view
    Vector3 v;
    v.x = (Float)viewX + (0.5f * p.x + 0.5f) * (Float)viewW;
    v.y = (Float)viewY + (0.5f * p.y + 0.5f) * (Float)viewH;
    v.z = 1.0f - (0.5f * p.z + 0.5f);

    //Find projected bounds
    if (c==0) bboxOut.min = bboxOut.max = v;
    else {

      if (v.x < bboxOut.min.x) bboxOut.min.x = v.x;
      if (v.y < bboxOut.min.y) bboxOut.min.y = v.y;
      if (v.z < bboxOut.min.z) bboxOut.min.z = v.z;

      if (v.x > bboxOut.max.x) bboxOut.max.x = v.x;
      if (v.y > bboxOut.max.y) bboxOut.max.y = v.y;
      if (v.z > bboxOut.max.z) bboxOut.max.z = v.z;
    }
  }

  return bboxOut;
}
