#include <app/app.h>

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

Convo *convo1 = NULL;
Convo *convo2 = NULL;

AnimController *animStandUp = NULL;
AnimController *animBossGreet = NULL;
AnimController *animSwitchSpeaker = NULL;


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

void keyDown (unsigned char key)
{
  switch (key)
  {
  case 13: //return

    if (convoCur == NULL)
      nextConvo();
    else
    {
      convoCur->skip();
      if (convoCur->done())
        nextConvo();
    }
    break;
  }
}

void animate()
{
  if (convoCur != NULL)
    convoCur->tick();
}

bool bindAnimByName (Scene3D *scene, AnimController *ctrl, const CharString &name)
{
  for (UintSize a=0; a<scene->animations.size(); ++a) {
    if (scene->animations[ a ]->name == name) {
      ctrl->bindAnimation( scene->animations[ a ] );
      return true; }}

  return false;
}

void onAnimRingStart (Convo *convo, ConvoNode *node)
{
  appPlayAnim( "StandUp" );
}

void onAnimBossGreet (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneOffice );
  appSwitchCamera( camOffice );
  appPlayAnim( "Greet" )->setNumLoops( 5 );
}

void onAnimSwitchSpeaker (Convo *convo, ConvoNode *node)
{
  appPlayAnim( "SwitchSpeaker" );
}

void onAnimLookEnable (Convo *convo, ConvoNode *node)
{
  appSwitchScene( sceneHouse );
  appSwitchCamera( camHouse );

  camHouseCtrl->enableLook( true );
  camHouseCtrl->enableMove( true );
  camHouse->lookAt( camHouse->getLook() );

  convo2->begin();
  convoCur = convo2;
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

  //Setup convo
  convo1 = new Convo;
  convo1->bindScene( scene2D );
  convos.pushBack( convo1 );

  convo1
    ->addSpeach( spkSound, 4.0f, "Rrrrrinnnng, Rrrrinnnnnnnnnnng!!!", onAnimRingStart )
    ->addSpeach( spkKlyde, 2.0f, "...err....Hello?" )
    ->addSpeach( spkBoss,  2.0f, "Klyde! Klyde! I can't believe we missed it!", onAnimBossGreet )
    ->addSpeach( spkBoss,  2.0f, "Urgh! Where were you yesterday, for god's sake?!" )
    ->addSpeach( spkKlyde, 3.0f, "Wait, what? Slow down, Boss. What's going on?" )
    ->addSpeach( spkBoss,  2.0f, "Haven't you read their paper this morning?" )
    ->addSpeach( spkBoss,  1.0f, "Oh, you gotta be kidding me!" )
    ->addSpeach( spkBoss,  2.0f, "Our competitors are totally killing us this week!" )
    ->addSpeach( spkBoss,  3.0f, "Hang on, I'll switch to speaker...", onAnimSwitchSpeaker, onAnimLookEnable );

  convo2 = new Convo;
  convo2->bindScene( scene2D );
  convos.pushBack( convo2 );

  convo2
    ->addSpeach( spkKlyde, 2.0f, "Umm... what are you talking about?" )
    ->addSpeach( spkKlyde, 3.0f, "You know we're still number one newspaper in town..." );
}

class EventObserver : public AnimObserver
{
public:
  virtual void onEvent (AnimEvent *evt)
  {
    std::cout << "Event '" << evt->getName().buffer() << "' triggered!" << std::endl;
  }
};

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

  //Find lights
  ArrayList<Actor*> lights;
  sceneHouse->findActorsByClass( Class(Light), lights );
  for (UintSize l=0; l<lights.size(); ++l)
  {
    //Set attenuation range
    Light* light = (Light*) lights[l];
    light->setAttenuation( 700.0f, 300.0f );
  }
  
  /////////////////////////////////////////////////
  //Setup 2D overlay

  scene2D = appScene2D();

  FpsLabel *lblFps = new FpsLabel;
  lblFps->setLoc( Vector2( 0.0f, (Float)resY-20 ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  lblFps->setParent( scene2D->getRoot() );
  
  //Create conversation
  initConvo();

  appRun();
  return EXIT_SUCCESS;
}
