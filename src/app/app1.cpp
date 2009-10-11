#include <app/app.h>
#include <engine/geGLHeaders.h>

int resX = 800;
int resY = 600;


Scene *scene2D;

Scene3D *sceneOffice;
Camera3D *camOffice;

Scene3D *sceneHouse;
Camera3D *camHouse;
FpsController *camHouseCtrl;

int convoIndex = -1;
Convo* convoCur = NULL;
ArrayList< Convo* > convos;

Convo *convoRing = NULL;
Convo *convoAnswer = NULL;
Convo *convo2 = NULL;
Convo *convoMug = NULL;

Actor3D *actMug = NULL;
Actor3D *actPaper = NULL;
Actor3D *actPhone = NULL;

bool stoodUp = false;
bool answeredPhone = false;
bool drankCoffee = false;
bool pickedPaper = false;

AnimController *animBright = NULL;

class BBoxWidget : public Widget
{
public:
  Actor3D *actor;
  
  virtual void draw()
  {
    if (actor == NULL) return;

    Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();
    BoundingBox bbox = appProjectActor( actor, camHouse, 0.0f, 0.0f, winSize.x, winSize.y );
    if (bbox.max.z < 0.0f) return;

    bbox.min.y = winSize.y - bbox.min.y;
    bbox.max.y = winSize.y - bbox.max.y;

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glColor3f( 1,1,1 );
    glBegin( GL_QUADS );
    glVertex2f( bbox.min.x, bbox.min.y );
    glVertex2f( bbox.max.x, bbox.min.y );
    glVertex2f( bbox.max.x, bbox.max.y );
    glVertex2f( bbox.min.x, bbox.max.y );
    glEnd();

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
};


void nextConvo ()
{
  if (convoIndex < (int)convos.size()-1)
  {
    if (convoCur != NULL)
      convoCur->end();

    convoIndex++;
    convoCur = convos[ convoIndex ];
    convoCur->begin();
  }
}

void beginConvo (Convo *convo)
{
  if (convoCur != NULL)
    convoCur->end();

  convoCur = convo;
  convoCur->begin();
}

void keyDown (unsigned char key)
{
  switch (key)
  {
  case 13: //return

    if (convoCur == NULL)
    {
      beginConvo( convoRing );
    }
    else
    {
      if (!convoCur->done())
        convoCur->skip();
    }

    /*
    if (convoCur == NULL)
      nextConvo();
    else
    {
      convoCur->skip();
      if (convoCur->done())
        nextConvo();
    }
    break;*/
  }
}

void animate()
{
  if (convoCur != NULL)
    convoCur->tick();

  if (!drankCoffee)
  {
    Matrix4x4 mugWorld = actMug->getGlobalMatrix();
    BoundingBox mugBBox = actMug->getBoundingBox();
    Vector3 mugCenter= mugWorld * mugBBox.center;
    
    Matrix4x4 camWorld = camHouse->getGlobalMatrix().affineNormalize();
    Vector3 camEye = camWorld.getColumn( 3 ).xyz();
    Vector3 camLook = camWorld.getColumn( 2 ).xyz();
    
    Vector3 eyeToMug = (mugCenter - camEye);
    Float dist = eyeToMug.norm();
    eyeToMug /= dist;

    Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();
    Float camFovHoriz = camHouse->getFov() * 0.5f * winSize.x/winSize.y;
    Float cosViewAngle = COS( Util::DegToRad( camFovHoriz ));
    Float cosMugAngle = Util::Max( Vector::Dot( eyeToMug, camLook ), 0.0f );
    Float dofCoeff = Util::Min( (1.0f - cosMugAngle) / (1.0f - cosViewAngle), 1.0f );
    dofCoeff = 1.0f - dofCoeff;

    DofParams dof = camHouse->getDofParams();
    dof.focusCenter = dist;
    dof.focusRange = dofCoeff * 50;
    dof.falloffFar = dofCoeff * 50;
    dof.falloffNear = dofCoeff * 50;
    camHouse->setDofParams( dof );
  }

  if (!pickedPaper)
  {

  }
}

void onAnimDrinkCoffee (Convo *convo, ConvoNode *node)
{

}

void onAnimStandUp (Convo *convo, ConvoNode *node)
{
  if (stoodUp) return;

  appPlayAnim( "StandUp" );
  camHouseCtrl->enableLook( false );
}

void onAnimStoodUp (Convo *convo, ConvoNode *node)
{
  if (stoodUp) return;
  stoodUp = true;

  camHouseCtrl->enableLook( true );
}

void onAnimBossGreet (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneOffice );
  appSwitchCamera( camOffice );
 
  appFinishAnim( "StandUp" );
  appPlayAnim( "CamLaptopSlide" )->setSpeed( 0.5f );
  appPlayAnim( "Greet" )->setNumLoops( 5 );
}

void onAnimSwitchSpeaker (Convo *convo, ConvoNode *node)
{
  appStopAnim( "Greet" );
  appPlayAnim( "SwitchSpeaker" );
}

void onAnimWalkToCabinet (Convo *convo, ConvoNode *node)
{
  appFinishAnim( "SwitchSpeaker" );
  appPlayAnim( "WalkToCabinet" );
}

void onAnimShakeHead1 (Convo *convo, ConvoNode *node)
{
  appFinishAnim( "WalkToCabinet" );
  appPlayAnim( "ShakeHead" );
  appPlayAnim( "CamAboveCabinet" );
}

void onAnimLookEnable (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneHouse );
  appSwitchCamera( camHouse );

  camHouseCtrl->enableLook( true );
  camHouseCtrl->enableMove( true );
  camHouse->lookAt( camHouse->getLook() );

  //beginConvo( convo2 );
}

void initConvo ()
{
  //camHouseCtrl->enableMove( false );
  //camHouseCtrl->enableLook( false );

  //Setup speakers
  ConvoSpeaker *spkSound = new ConvoSpeaker;
  spkSound->color = Vector3( 1, 0.8f, 0 );

  ConvoSpeaker *spkKlyde = new ConvoSpeaker;
  spkKlyde->color = Vector3( 0,1,0 );

  ConvoSpeaker *spkBoss = new ConvoSpeaker;
  spkBoss->color = Vector3( 1,0,0 );

  //Looping ring convo
  convoRing = new Convo;
  convoRing->bindScene( scene2D );
  convos.pushBack( convoRing );

  ConvoSpeach *convoRingStart = convoRing
    ->addSpeach( spkSound, 4.0f, "Rrrrrinnnng, Rrrrinnnnnnnnnnng!!!", onAnimStandUp, onAnimStoodUp );
  
  ConvoSpeach *convoRingEnd = convoRingStart
    ->addSpeach( spkSound, 4.0f, "" );

  convoRingEnd->next = convoRingStart;

  //Intro boss convo
  convoAnswer = new Convo;
  convoAnswer->bindScene( scene2D );
  convos.pushBack( convoAnswer );
  convoAnswer
    ->addSpeach( spkKlyde, 2.0f, "...err....Hello?" )
    ->addSpeach( spkBoss,  2.0f, "Klyde! Klyde! I can't believe we missed it!", onAnimBossGreet )
    ->addSpeach( spkBoss,  2.0f, "Urgh! Where were you yesterday, for god's sake?!" )
    ->addSpeach( spkKlyde, 3.0f, "Wait, what? Slow down, Boss. What's going on?" )
    ->addSpeach( spkBoss,  2.0f, "Haven't you read their paper this morning?" )
    ->addSpeach( spkBoss,  1.0f, "Oh, you gotta be kidding me!" )
    ->addSpeach( spkBoss,  2.0f, "Our competitors are totally killing us this week!" )
    ->addSpeach( spkBoss,  3.0f, "Hang on, I'll switch to speaker...", onAnimSwitchSpeaker, onAnimWalkToCabinet )
    ->addSpeach( spkKlyde, 2.0f, "Umm... what are you talking about?" )
    ->addSpeach( spkKlyde, 3.0f, "You know we're still number one newspaper in town..." )
    ->addSpeach( spkBoss,  3.0f, "What’s wrong with you? Pull it together, man!", onAnimShakeHead1 )
    ->addSpeach( spkBoss,  3.0f, "I want you to have a look at the newspaper right now!", NULL, onAnimLookEnable )
    ->addSpeach( spkKlyde, 4.0f, "Oooookay… Gimme a second, should be at the front door..." );

  //...
  convo2 = new Convo;
  convo2->bindScene( scene2D );
  convos.pushBack( convo2 );


  convoMug = new Convo;
  convoMug->bindScene( scene2D );
  convoMug->addSpeach( spkSound, 2.0f,
    "Yumm, yumm, yummmmmm....mmmm!", onAnimDrinkCoffee );
}

class EventObserver : public AnimObserver
{
public:
  virtual void onEvent (AnimEvent *evt)
  {
    std::cout << "Event '" << evt->getName().buffer() << "' triggered!" << std::endl;
  }
};

typedef void (*MaterialFunc) (Actor3D *actor, StandardMaterial *mat);

void forEachMaterial (Actor3D *actor, MaterialFunc func)
{
  Material *mat = actor->getMaterial();
  if (ClassOf( mat ) == Class( MultiMaterial ))
  {
    MultiMaterial *mm = (MultiMaterial*) mat;
    for (UintSize m=0; m < mm->getNumSubMaterials(); ++m)
    {
      Material *submat = mm->getSubMaterial( (MaterialID) m );
      func( actor, (StandardMaterial*) submat );
    }
  }
  else func( actor, (StandardMaterial*) mat );
}

void highlight (Actor3D *actor, StandardMaterial *material) {
  material->setLuminosity( 0.3f );
}

void unhighlight (Actor3D *actor, StandardMaterial *material) {
  material->setLuminosity( 0.0f );
}

void onMouseHighlight (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  if (evt == ActorMouseEvent::Enter) {
    forEachMaterial( actor, highlight );
  }
  else if (evt == ActorMouseEvent::Leave) {
    forEachMaterial( actor, unhighlight );
  }
}

void onClickMug (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  drankCoffee = true;
  camHouse->setDofEnabled( false );
  beginConvo( convoMug );
}

void onClickPaper (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  if (pickedPaper) return;
  pickedPaper = true;

  //Extract paper scale
  Matrix4x4 &mat = actor->getMatrix();
  Float scale = mat.getColumn(0).xyz().norm();

  //Extract camera scale
  Matrix4x4 matCam = camHouse->getGlobalMatrix();
  Float camScale = matCam.getColumn(0).xyz().norm();

  //Attach relative to camera and reapply scale
  actor->setParent( camHouse );
  actor->setMatrix( Matrix4x4() );
  actor->scale( scale );
  actor->rotate( Vector3( 1, 0, 0 ), Util::DegToRad( -70 ) );
  actor->rotate( Vector3( 0, 0, 1 ), Util::DegToRad( 10 ) );
  actor->translate( Vector3( -35.0f, -20.0f, 100.0f ) / camScale );
  sceneHouse->updateChanges();
}

void onClickPhone (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  if (answeredPhone) return;
  answeredPhone = true;
  beginConvo( convoAnswer );
}

//TODO: Stuf doesn't load properly if a class used by the exporter is not
//used in the application and therefore it doesn't get classified in runtime!!!
ActorAnimObserver a;
SkinAnimObserver b;

int main (int argc, char **argv)
{
  //Initialize GLUT
  appInit( resX, resY );
  appAnimateFunc( animate );
  appKeyDownFunc( keyDown );

  /////////////////////////////////////////////////
  //Setup office scene

  sceneOffice = appScene3D( "BossOffice.pak" );
  if (sceneOffice == NULL) {
    std::cout << "Failed loading scene file!" << std::endl;
    std::getchar();
    return EXIT_FAILURE;
  }

  //Find first camera in the scene
  camOffice = (Camera3D*) sceneOffice->findFirstActorByClass( Class(Camera3D) );

  /////////////////////////////////////////////////
  //Setup house scene

  sceneHouse = appScene3D( "WakingUp.pak" );
  if (sceneHouse == NULL) {
    std::cout << "Failed loading scene file!" << std::endl;
    std::getchar();
    return EXIT_FAILURE;
  }

  //Find first camera in the scene
  camHouse = (Camera3D*) sceneHouse->findFirstActorByClass( Class(Camera3D) );
  camHouseCtrl = appCamCtrl( camHouse );
  //camHouseCtrl->setClickToLook( false );

  //Find important actors in the scene
  actPhone = (Actor3D*) sceneHouse->findFirstActorByName( "Telephone" );
  actMug = (Actor3D*) sceneHouse->findFirstActorByName( "Mug" );
  actPaper = (Actor3D*) sceneHouse->findFirstActorByName( "Newspaper" );
  
  appActorMouseFunc( ActorMouseEvent::Enter, actPhone, onMouseHighlight );
  appActorMouseFunc( ActorMouseEvent::Enter, actMug, onMouseHighlight );
  appActorMouseFunc( ActorMouseEvent::Enter, actPaper, onMouseHighlight );
  
  appActorMouseFunc( ActorMouseEvent::Click, actPhone, onClickPhone);
  appActorMouseFunc( ActorMouseEvent::Click, actMug, onClickMug );
  appActorMouseFunc( ActorMouseEvent::Click, actPaper, onClickPaper );

  Actor3D* sky = (Actor3D*) sceneOffice->findFirstActorByName( "Sky" );
  ((StandardMaterial*)sky->getMaterial())->setLuminosity( 2.0f );
  
  /////////////////////////////////////////////////
  //Setup 2D overlay

  scene2D = appScene2D();

  BBoxWidget *w = new BBoxWidget;
  w->actor = actPhone;
  //scene2D->getRoot()->addChild( w );

  FpsLabel *lblFps = new FpsLabel;
  lblFps->setLoc( Vector2( 0.0f, (Float)resY-20 ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  lblFps->setParent( scene2D->getRoot() );

  /////////////////////////////////////////////////
  //Brightness animation
  
  //Create conversation
  initConvo();

  appRun();
  return EXIT_SUCCESS;
}
