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
Convo *convoPaper = NULL;
Convo *convoMug = NULL;
Convo *convoInfo = NULL;

Actor3D *actMug = NULL;
Actor3D *actPaper = NULL;
Actor3D *actPhone = NULL;

bool stoodUp = false;
bool answeredPhone = false;
bool talkedToBoss = false;
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

  if (talkedToBoss && !drankCoffee)
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
    Float cosViewAngle = COS( Util::DegToRad( camFovHoriz + 10.0f ));
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

void onAnimStandUp (Convo *convo, ConvoNode *node)
{
  if (stoodUp) return;

  appPlayAnim( "StandUp" );
  camHouseCtrl->enableLook( false );
  camHouseCtrl->enableMove( false );
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
  appFinishAnim( "CamLaptopSlide" );
  appFinishAnim( "Greet" );
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
  appPlayAnim( "CamBesideCabinet" );
  appPlayAnim( "ShakeHead" );
}

void onAnimPourAndDrink1 (Convo *convo, ConvoNode *node)
{
  appFinishAnim( "ShakeHead" );
  appPlayAnim( "PourAndDrink" );
}

void onAnimLookEnable (Convo *convo, ConvoNode *node)
{
  talkedToBoss = true;

  appSwitchScene( sceneHouse );
  appSwitchCamera( camHouse );

  camHouseCtrl->enableLook( true );
  camHouseCtrl->enableMove( true );
  camHouse->lookAt( camHouse->getLook() );
}

void onAnimShakeHead2  (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneOffice );
  appSwitchCamera( camOffice );

  appPlayAnim( "CamAboveCabinet" );
  appPlayAnim( "ShakeHead" );
}

void onAnimPourAndDrink2 (Convo *convo, ConvoNode *node)
{
  appFinishAnim( "ShakeHead" );
  appPlayAnim( "PourAndDrink" );
}

void onAnimNeedCoffee (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneHouse );
  appSwitchCamera( camHouse );
}

void onAnimGetCoffee (Convo *convo, ConvoNode *node)
{
  camHouseCtrl->enableMove( true );
}

void onAnimDrankCoffee (Convo *convo, ConvoNode *node)
{
  if (pickedPaper)
    beginConvo( convoInfo );
}

void onAnimBackToChair (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneOffice );
  appSwitchCamera( camOffice );

  appPlayAnim( "BackToChair" );
}

void onAnimIntercom (Convo *convo, ConvoNode *node)
{
  Kernel::GetInstance()->getRenderer()->setAvgLuminance( 0.3f );
  camOffice->setDofEnabled( false );
  appPlayAnim( "CamIntercom" );
  appPlayAnim( "Intercom" )->setNumLoops( 1 );
}

void onAnimMarlene (Convo *convo, ConvoNode *node)
{
  camOffice->setDofEnabled( false );
  appPlayAnim( "Marlene" );
}

void initConvo ()
{
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
    ->addSpeach( spkKlyde, 6.0f, "You know we're still number one newspaper in town..." )
    ->addSpeach( spkBoss,  3.0f, "What’s wrong with you? Pull it together, man!", onAnimShakeHead1 )
    ->addSpeach( spkBoss,  4.0f, "I want you to have a look at today's issue right now!", onAnimPourAndDrink1, onAnimLookEnable )
    ->addSpeach( spkKlyde, 4.0f, "Oooookay… Gimme a second, I think I left it on the sofa..." )
    ->addSpeach( spkKlyde, 2.0f, "" )
    ->addSpeach( spkKlyde, 3.0f, "Urgh... so sleeepy!" );

  //After picking up the paper
  convoPaper = new Convo;
  convoPaper->bindScene( scene2D );
  convos.pushBack( convoPaper );

  ConvoBranch *branchPaper1 =
  convoPaper
    ->addSpeach( spkKlyde, 3.0f, "Alright, let's see... Did you mean..." )
    ->addBranch( -1.0f );
  
  ConvoSpeach *wrongAnswer =
    new ConvoSpeach( spkBoss, 4.0f, "No, no! The one about the GIRL, idiot! Should be on the front page." );

  branchPaper1->addOption( "Toilet Seat" )
    ->addSpeach( spkKlyde, 4.0f, "Flaming toilet seat causes evacuation at high school..." )
    ->next = wrongAnswer;

  branchPaper1->addOption( "Horse Suit" )
    ->addSpeach( spkKlyde, 4.0f, "Doctor testifies in horse suit..." )
    ->next = wrongAnswer;

  branchPaper1->addOption( "Iraqi Head" )
    ->addSpeach( spkKlyde, 4.0f, "Iraqi head seeks arms..." )
    ->next = wrongAnswer;

  branchPaper1->addOption( "Jet Crash" )
    ->addSpeach( spkKlyde, 4.0f, "Something went wrong in jet crash, expert says..." )
    ->next = wrongAnswer;

  ConvoBranch *branchPaper2 = wrongAnswer->addBranch( -1.0f );

  ConvoSpeach *girlAnswer = 
    new ConvoSpeach( spkBoss, 4.0f, "No, you moron, you're looking at the back page! Wake up already!",
    onAnimShakeHead2, onAnimPourAndDrink2 );

  branchPaper2->addOption( "Call me, I'm all yours!" )
    ->addSpeach( spkKlyde, 3.0f, "Call me, I'm all yours!" )
    ->next = girlAnswer;

  branchPaper2->addOption( "Naughty Marlene needs a daddy!" )
    ->addSpeach( spkKlyde, 3.0f, "Naughty Marlene needs a daddy!" )
    ->next = girlAnswer;

  branchPaper2->addOption( "Heartbroken next door!" )
    ->addSpeach( spkKlyde, 3.0f, "Heartbroken next door!" )
    ->next = girlAnswer;

  girlAnswer
    ->addSpeach( spkBoss, 2.0f, "", NULL, onAnimNeedCoffee )
    ->addSpeach( spkKlyde, 3.0f, "Hang on... I really need some caffeine." )
    ->addSpeach( spkBoss, 2.0f, "Jeez, be quick about it!", NULL, onAnimGetCoffee );


  //When drinking coffee
  convoMug = new Convo;
  convoMug->bindScene( scene2D );
  convoMug->addSpeach( spkSound, 2.0f,
    "Yumm, yumm, yummmmmm....mmmm!", NULL, onAnimDrankCoffee );

  //After having drank coffee and picked up the newspaper
  convoInfo = new Convo;
  convoInfo->bindScene( scene2D );
  convos.pushBack( convoInfo );

  ConvoBranch *branchInfo =
  convoInfo
    ->addSpeach( spkKlyde, 1.5f, "Let's see..." )
    ->addSpeach( spkKlyde, 3.0f, "Oh my god, they finally got a lead on the VeraSign conspiracy!" )
    ->addBranch( -1.0f );

  branchInfo
    ->addOption( "VeraSign" )
    ->addSpeach( spkKlyde, 4.0f, "There was always something fishy about those AlterShell suits they make." )
    ->addSpeach( spkBoss, 3.0f, "Yeah but still everyone loves them." )
    ->addSpeach( spkBoss, 2.0f, "Being healthy and looking good... How can you get better than that?" )
    ->addSpeach( spkBoss, 3.0f, "All that from one suit... it's awesome man!" )
    ->next = branchInfo;

  branchInfo
    ->addOption( "Conspiracy" )
    ->addSpeach( spkKlyde, 3.0f, "Well it's about time. I kept hearing these rumors " )
    ->addSpeach( spkKlyde, 3.0f, "about the missing people being connected to VeraSign." )
    ->addSpeach( spkBoss, 3.0f, "Yeah and apparently the police finally have a lead." )
    ->addSpeach( spkBoss, 3.0f, "Then again, there's those protestors who hate AlterShells." )
    ->addSpeach( spkBoss, 3.0f, "Maybe THEY made it all up, who knows." )
    ->next = branchInfo;

  branchInfo
    ->addOption( "Innocent" )
    ->addSpeach( spkKlyde, 3.0f, "Oh wow, so they've connected a girl to one of the missing people." )
    ->addSpeach( spkBoss, 4.0f, "Yup but apparently none of the missing person's relatives recognize her." )
    ->addSpeach( spkBoss, 3.0f, "She sounds innocent to me. I mean..." )
    ->addSpeach( spkBoss, 3.0f, "...they did confirm she wasn't wearing an avatar." )
    ->next = branchInfo;

  branchInfo
    ->addOption( "Get to work" )
    ->addSpeach( spkKlyde, 3.0f, "So you want me to cover this story, right?", NULL, onAnimBackToChair )
    ->addSpeach( spkBoss, 4.0f, "Sure thing. I want a thousand words and I want it on my desk by tonight!" )
    ->addSpeach( spkBoss, 3.0f, "And no drinking with your mates until your're done!" )
    ->addSpeach( spkBoss, 4.0f, "I'll be down at the bar making sure you're not slacking off." )
    ->addSpeach( spkBoss, 3.0f, "KAPISH?!" )
    ->addSpeach( spkSound, 1.0f, "*Click*" )
    ->addSpeach( spkBoss, 1.0f, "", onAnimIntercom )
    ->addSpeach( spkBoss, 3.0f, "Darling, can you get me my coat, please?" )
    ->addSpeach( spkBoss, 2.0f, "I have a meeting with..." )
    ->addSpeach( spkBoss, 5.0f, "Marlene.", onAnimMarlene );

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
  if (!answeredPhone) return;
  if (drankCoffee) return;
  drankCoffee = true;

  //Remove depth of field lock
  camHouse->setDofEnabled( false );
  beginConvo( convoMug );
}

void onClickPaper (ActorMouseEvent::Enum evt, Actor3D *actor)
{
  if (!answeredPhone) return;
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

  if (drankCoffee)
    beginConvo( convoInfo );
  else
  {
    beginConvo( convoPaper );
    camHouseCtrl->enableMove( false );
  }
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

  sceneOffice = appScene3D( "Export/BossOffice.pak" );
  if (sceneOffice == NULL) {
    std::cout << "Failed loading scene file!" << std::endl;
    std::getchar();
    return EXIT_FAILURE;
  }

  //Find first camera in the scene
  camOffice = (Camera3D*) sceneOffice->findFirstActorByClass( Class(Camera3D) );
  //camOffice->setDofEnabled( true );

  /////////////////////////////////////////////////
  //Setup house scene

  sceneHouse = appScene3D( "Export/WakingUp.pak" );
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
