#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
#include <ui/uiUI.h>
using namespace GE;

#include <cstdlib>
#include <cstdio>
#include <iostream>


namespace CameraMode
{
  enum Enum
  {
    Pan,
    Orbit,
    Zoom
  };
}

CharString animName;
Float animSpeed = 1.0f;

UI::Window *window = NULL;
FpsLabel *lblFps = NULL;

ByteString data;
TriMesh *mesh = NULL;
Character *character = NULL;
TriMeshActor *triMeshActor = NULL;
SkinMeshActor *skinMeshActor = NULL;
Actor *actorRender = NULL;

Light *light = NULL;

Scene *scene = NULL;
Scene *sceneSky = NULL;
Scene *sceneRender = NULL;

FpsController *ctrl = NULL;
Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camSky = NULL;
Camera3D *camRender = NULL;
CameraMode::Enum cameraMode;

Renderer *renderer = NULL;

int resX = 800;
int resY = 600;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );



/*
-------------------------------------------
UI Events
-------------------------------------------*/

class MouseEvent : public UI::Event
{
  DECLARE_SUBCLASS( MouseEvent, UI::Event );
  DECLARE_END;

public:
  int x;
  int y;
};

class ClickEvent : public MouseEvent
{
  DECLARE_SUBCLASS( ClickEvent, MouseEvent );
  DECLARE_END;

public:
  int button;
  int state;
};

class MouseLeaveEvent : public MouseEvent
{
  DECLARE_SUBCLASS( MouseLeaveEvent, MouseEvent );
  DECLARE_END;
};

class MouseEnterEvent : public MouseEvent
{
  DECLARE_SUBCLASS( MouseEnterEvent, MouseEvent );
  DECLARE_END;
};


DEFINE_CLASS( MouseEvent );
DEFINE_CLASS( ClickEvent );
DEFINE_CLASS( MouseEnterEvent );
DEFINE_CLASS( MouseLeaveEvent );


class ClickPin : public UI::Pin
{
public:
  ClickEvent eClick;

  void bothPins (UI::Widget *w)
  {
    eClick.trigger( w );
  };
};

class MouseFocusPin : public UI::Pin
{
public:
  MouseEnterEvent eEnter;
  MouseLeaveEvent eLeave;

  void thisPin (UI::Widget *w)
  {
    eEnter.trigger( w );
  }

  void otherPin (UI::Widget *w)
  {
    eLeave.trigger( w );
  }
};

class UICtrl
{
  ClickPin pMouseClick[2];
  bool mouseDown;

  MouseFocusPin pMouseFocus[2];
  Int iMouseFocus;

public:

  UICtrl()
  {
    iMouseFocus = 0;
    mouseDown = false;
  }
  
  void mouseClick (int button, int state, int x, int y)
  {
    if (button != GLUT_LEFT_BUTTON) return;

    UI::Widget *top = window->findTopWidgetAt( x, y );

    if (state == GLUT_DOWN)
    {
      pMouseClick[0].drop( top );
      mouseDown = true;
    }

    if (state == GLUT_UP)
    {
      pMouseClick[1].eClick.button = button;
      pMouseClick[1].eClick.state = state;
      pMouseClick[1].eClick.x = x;
      pMouseClick[1].eClick.y = y;

      pMouseClick[1].drop( top );
      pMouseClick[1].intersect( pMouseClick[0] );

      mouseDown = false;
      mouseMove( x, y );
    }
  }

  void mouseMove (int x, int y)
  {
    if (!mouseDown)
    {
      UI::Widget *top = window->findTopWidgetAt( x, y );
      
      int iNew = (iMouseFocus+1) % 2;
      pMouseFocus[ iNew ].drop( top );
      pMouseFocus[ iNew ].intersect( pMouseFocus[ iMouseFocus ] );
      iMouseFocus = iNew;
    }
  }
};

class Convo;
class ConvoOpt;

class ConvoOptLabel : public Label
{
public:

  Vector3 outColor;
  Vector3 inColor;
  Convo *convo;
  ConvoOpt *opt;

  void onEvent (UI::Event *e)
  {
    ClickEvent *eClick = SafeCast( ClickEvent, e );
    if (eClick != NULL) onClick( eClick );

    MouseEnterEvent *eEnter = SafeCast( MouseEnterEvent, e );
    if (eEnter != NULL) onMouseEnter( eEnter );

    MouseLeaveEvent *eLeave = SafeCast( MouseLeaveEvent, e );
    if (eLeave != NULL) onMouseLeave( eLeave );
  }

  void onClick (ClickEvent *e);

  void onMouseEnter (MouseEnterEvent *e)
  {
    setColor( inColor);
  }

  void onMouseLeave (MouseLeaveEvent *e)
  {
    setColor( outColor );
  }
};


/*
-------------------------------------------
Conversation
-------------------------------------------*/

class ConvoNode;
class ConvoSpeach;
class ConvoBranch;
class ConvoOpt;
class ConvoSpeaker;


class ConvoNode : public Object
{
  DECLARE_SUBCLASS( ConvoNode, Object );
  DECLARE_END;

public:
  Float duration;
  ConvoNode *next;
  ConvoNode() : next(NULL), duration(0.0) {}
};

class ConvoSpeach : public ConvoNode
{
  DECLARE_SUBCLASS( ConvoSpeach, ConvoNode );
  DECLARE_END;

public:
  ConvoSpeaker *speaker;
  CharString text;

  ConvoSpeach() : speaker(NULL) {}
};

class ConvoBranch : public ConvoNode
{
  DECLARE_SUBCLASS( ConvoBranch, ConvoNode );
  DECLARE_END;

public:
  ArrayList <ConvoOpt*> options;
};

class ConvoOpt
{
public:
  CharString text;
  ConvoNode *next;
};

class ConvoSpeaker
{
public:
  Vector2 loc;
  Vector3 color;
};

class Convo : public ConvoNode
{
  Float time;
  ConvoNode *node;
  UI::Widget *widget;
  void show();
  void hide();

public:
  void advance (ConvoNode *to);
  void begin();
  void tick();

  Convo() : node(NULL), widget(NULL) {}
};

DEFINE_CLASS( ConvoNode );
DEFINE_CLASS( ConvoSpeach );
DEFINE_CLASS( ConvoBranch );

void Convo::begin()
{
  node = next;
  show();
}

void Convo::show()
{
  if (node == NULL)
    return;

  if (ClassOf( node ) == Class( ConvoSpeach ))
  {
    ConvoSpeach* speach = (ConvoSpeach*) node;

    Label *lbl = new Label;
    lbl->setText( speach->text );
    lbl->setLoc( 10, resY - 100 );
    lbl->setColor( speach->speaker->color );
    lbl->setParent( window );

    widget = lbl;
  }
  else if (ClassOf( node ) == Class( ConvoBranch ))
  {
    ConvoBranch* branch = (ConvoBranch*) node;

    UI::Widget *grp = new UI::Widget;
    grp->setParent( window );

    for (UintSize o=0; o<branch->options.size(); ++o)
    {
      ConvoOpt* opt = branch->options[o];

      ConvoOptLabel *lbl = new ConvoOptLabel;
      lbl->opt = opt;
      lbl->convo = this;
      lbl->outColor = Vector3(1,1,1);
      lbl->inColor = Vector3(1,1,0);
      lbl->setText( opt->text );
      lbl->setLoc( 10, 20 + o * 20 );
      lbl->setColor( lbl->outColor );
      lbl->setParent( grp );
    }

    widget = grp;
  }

  time = Kernel::GetInstance()->getTime();
}

void Convo::hide()
{
  if (widget != NULL)
    widget->destroy();
}

void Convo::advance (ConvoNode *to)
{
  hide();
  node = to;
  show();
}

void Convo::tick()
{
  if (node == NULL)
    return;

  Float newTime = Kernel::GetInstance()->getTime();
  if (newTime - time > node->duration)
    advance( node->next );
}

void ConvoOptLabel::onClick (ClickEvent *e)
{
  convo->advance( opt->next );
}

Convo *convo = NULL;
UICtrl *uictrl = NULL;

/*
-------------------------------------------
Driver
-------------------------------------------*/

//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}

void click (int button, int state, int x, int y)
{
  //ctrl->mouseClick( button, state, x, y );
  uictrl->mouseClick( button, state, x, y );
}

void drag (int x, int y)
{
  //ctrl->mouseMove( x, y );
  uictrl->mouseMove( x, y );
}

void keyDown (unsigned char key, int x, int y)
{
  switch (key)
  {
  case 9://tab
    renderer->setIsDofEnabled( !renderer->getIsDofEnabled() );
    break;

  case 8://backspace
    if (skinMeshActor == NULL) return;
    skinMeshActor->loadPose();
    break;
  /*
    if (skinMeshActor == NULL) return;
    skinMeshActor->playAnimation( animName, animSpeed );
    break;
  */
  case 13://return
    if (skinMeshActor == NULL) return;
    if (!skinMeshActor->isAnimationPlaying())
      skinMeshActor->loopAnimation( animName, animSpeed );
    else skinMeshActor->pauseAnimation();
    break;

  case '+':
    if (skinMeshActor == NULL) return;
    animSpeed += 0.1f;
    skinMeshActor->setAnimationSpeed( animSpeed );
    printf ("Animation speed: %f\n", animSpeed );
    break;

  case '-':
    if (skinMeshActor == NULL) return;
    if (animSpeed > 0.11f) animSpeed -= 0.1f;
    skinMeshActor->setAnimationSpeed( animSpeed );
    printf ("Animation speed: %f\n", animSpeed );
    break;

  case 27:
    //Quit on escape
    exit(0);
  }

  ctrl->keyDown( key );
}

void keyUp (unsigned char key, int x, int y)
{
  ctrl->keyUp( key );
}

void specialKey (int key, int x, int y)
{
  float l;
  float lstep = 0.05f;
  float fstep = 10.0f;
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


void updateSkyCamera()
{
  Matrix4x4 mSky = camRender->getMatrix();
  mSky.setColumn( 3, camSky->getMatrix().getColumn(3) );
  camSky->setMatrix( mSky );
}

void display ()
{
  updateSkyCamera();

  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->beginFrame();
  
  //draw model
  renderer->beginDeferred();
  renderer->setCamera( camRender );
  renderer->renderSceneDeferred( sceneRender );
  renderer->endDeferred();
  
  //Frames per second
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( cam2D );
  renderer->renderWindow( window );
  
  renderer->endFrame();
}

void reshape (int w, int h)
{
  resX = w;
  resY = h;

  if (renderer != NULL)
    renderer->setWindowSize( w,h );
}

void findBounds (TriMesh *mesh, const Matrix4x4 xform)
{
  VertexBinding <TriVertex> vertBind;
  vertBind.init( mesh->getFormat() );
  TriVertex vert;

  if (mesh->getVertexCount() > 0) {
    vert = vertBind( mesh->getVertex(0) );
    center = boundsMin = boundsMax = xform * (*vert.coord);
  }

  for (UintSize v=1; v<mesh->getVertexCount(); ++v)
  {
    TriVertex vert = vertBind( mesh->getVertex( v ) );
    Vector3 point = xform * (*vert.coord);
    center += point;

    if (point.x < boundsMin.x) boundsMin.x = point.x;
    if (point.x > boundsMax.x) boundsMax.x = point.x;
    if (point.y < boundsMin.y) boundsMin.y = point.y;
    if (point.y > boundsMax.y) boundsMax.y = point.y;
    if (point.z < boundsMin.z) boundsMin.z = point.z;
    if (point.z > boundsMax.z) boundsMax.z = point.z;
  }
  
  if (mesh->getVertexCount() > 0)
    center /= mesh->getVertexCount();
}

void cleanup()
{
}

void animate()
{
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );
  
  if (skinMeshActor != NULL)
    skinMeshActor->tick();

  ctrl->tick();
  convo->tick();

  glutPostRedisplay();
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Demo" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyDown );
  glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  glutMouseFunc( click );
  glutMotionFunc( drag );
  glutPassiveMotionFunc( drag );
  glutIdleFunc( animate );
  idleDraw = true;
}

bool loadPackage (const CharString &fileName)
{
  //Read the file
  File file( fileName );
  if( !file.open( "rb" )) {
    printf( "Failed opening file '%s'!\n", fileName.buffer() );
    return false;
  }

  //Read signature
  SerializeManager sm;
  ByteString sig = file.read( sm.getSignatureSize() );
  if (sig.length() != sm.getSignatureSize()) {
    printf( "Missing file signature!\n" );
    file.close();
    return false;
  }

  //Check signature
  if ( ! sm.checkSignature( sig.buffer() )) {
    printf( "Invalid file signature!\n" );
    file.close();
    return false;
  }
  
  //Read the rest of the file
  data = file.read( file.getSize() - sm.getSignatureSize() );
  file.close();

  //Deserialize
  ClassPtr cls;
  void *object = sm.load( (void*)data.buffer(), &cls );

  //Check object class
  if (cls == Class(TriMesh))
  {
    mesh = (TriMesh*) object;
    mesh->sendToGpu();

    //Report
    printf ("Static mesh: %d verts, %d faces\n",
            mesh->getVertexCount(),
            mesh->getFaceCount());
  }
  else if (cls == Class(Character))
  {
    character = (Character*) object;
    
    //Mesh report
    printf ("Character: %d verts, %d faces, %d animations\n",
            character->mesh->getVertexCount(),
            character->mesh->getFaceCount(),
            character->anims.size());
    
    //Animations report
    for (UintSize a=0; a<character->anims.size(); ++a)
      printf ("Animation [%d]: '%s'\n", a,
              character->anims[ a ]->name.buffer());

    //Get animation name
    if (character->anims.size() > 0 && animName == "") {
      char buf[256]; int len=0; UintSize animIndex=0;
      while (len==0 || animIndex >= character->anims.size())
      {
        std::cout << "Pick animation: ";
        std::cin.width( 256 );
        std::cin >> buf;
        CharString str = buf;
        animIndex = str.parseIntegerAt(0, &len);
      }
      animName = character->anims[ animIndex ]->name;
    }
    
    //Split into 24-bone sub meshes
    SkinSuperToSubMesh splitter( character->mesh );
    splitter.splitByBoneLimit( 24 );

    UintSize numMeshes = splitter.getSubMeshCount();
    for (UintSize m=0; m<numMeshes; ++m)
    {
      SkinTriMesh *mesh = (SkinTriMesh*) splitter.getSubMesh(m);
      character->meshes.pushBack( mesh );
      mesh->sendToGpu();
    }
  }

  return true;
}

Actor* loadActor (const CharString &meshFileName,
                  const CharString &texFileName="",
                  const CharString &normTexFileName="")
{
  mesh = NULL;
  character = NULL;
  actorRender = NULL;

  if (!loadPackage( meshFileName )) {
    std::cout << "Failed loading '" << meshFileName.buffer() << std::endl;
    std::getchar();
    exit(0);
    return NULL;
  }

  //Create actor
  TriMesh *meshRender = NULL;
  
  if (mesh != NULL)
  {
    triMeshActor = new TriMeshActor;
    triMeshActor->setMesh( mesh );
    meshRender = mesh;
    actorRender = triMeshActor;
  }

  if (character != NULL)
  {
    skinMeshActor = new SkinMeshActor;
    skinMeshActor->setMesh( character );
    meshRender = character->mesh;
    actorRender = skinMeshActor;
  }

  //Center in the scene
  findBounds( meshRender, actorRender->getWorldMatrix() );
  Vector3 trans = center * -1;
  actorRender->translate( trans.x, trans.y, trans.z );

  //Scale to [100,100,100] range
  Vector3 size = boundsMax - boundsMin;
  Float sizemax = Util::Max( Util::Max( size.x, size.y), size.z );
  Float scale = 100.0f / sizemax;
  actorRender->scale( scale );
  findBounds( meshRender, actorRender->getWorldMatrix() );

  if (normTexFileName != "")
  {
    //Assing normal-texture material
    Image *imgDiff = new Image;
    imgDiff->readFile( texFileName, "jpeg" );

    Image *imgNorm = new Image;
    imgNorm->readFile( normTexFileName, "jpeg" );

    Texture *texDiff = new Texture;
    texDiff->fromImage( imgDiff );

    Texture *texNorm = new Texture;
    texNorm->fromImage( imgNorm );

    NormalTexMat *matTex = new NormalTexMat;
    matTex->setSpecularity( 0.5 );
    matTex->setDiffuseTexture( texDiff );
    matTex->setNormalTexture( texNorm );
    actorRender->setMaterial( matTex );
  }
  else if (texFileName != "")
  {
    //Assing diffuse-texture material
    Image *img = new Image;
    img->readFile( texFileName, "jpeg" );

    Texture *tex = new Texture;
    tex->fromImage( img );

    DiffuseTexMat *matTex = new DiffuseTexMat;
    //matTex->setSpecularity( 0.5 );
    matTex->setDiffuseTexture( tex );
    actorRender->setMaterial( matTex );
  }
  else
  {
    //Assign solid color material
    StandardMaterial *matWhite = new StandardMaterial;
    //matWhite->setSpecularity( 0.5 );
    actorRender->setMaterial( matWhite );
  }

  return actorRender;
}

Material* loadTexMaterial (const CharString &texFileName)
{
  Image *img = new Image;
  ImageErrorCode err = img->readFile( texFileName, "png" );
  if (err != IMAGE_NO_ERROR)
    std::cout << "Problems while loading '" << texFileName.buffer() << "'!" << std::endl;

  Texture *tex = new Texture;
  tex->fromImage( img );

  DiffuseTexMat *mat = new DiffuseTexMat;
  mat->setDiffuseTexture( tex );
  mat->setLuminosity( 0.05f );
  return mat;
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );

  //Setup depth-of-field
  renderer->setDofParams( 200, 50, 150, 150 );

  //Setup 3D scene
  sceneSky = new Scene;
  scene = new Scene;
/*
  MultiMaterial *mm = new MultiMaterial;
  mm->setNumSubMaterials( 24 );
  mm->setSubMaterial( 0, loadTexMaterial( "Textures/BlackWoodToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 0 ))->setSpecularity( 1.0f );
  ((StandardMaterial*)mm->getSubMaterial( 0 ))->setGlossiness( 0.2f );
  mm->setSubMaterial( 1, loadTexMaterial( "Textures/WoodFloor.png" ) );
  mm->setSubMaterial( 2, loadTexMaterial( "Textures/DarkBrickToon.png" ) );
  mm->setSubMaterial( 3, loadTexMaterial( "Textures/WhiteTileToon4x4.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 3 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 4, loadTexMaterial( "Textures/BeigePlasterToon.png" ) );
  mm->setSubMaterial( 5, new StandardMaterial() );
  mm->setSubMaterial( 6, loadTexMaterial( "Textures/HouseWood5Toon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 6 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 7, loadTexMaterial( "Textures/FrostedGlass.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 7 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 8, loadTexMaterial( "Textures/BrushedMetal.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 8 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 9, loadTexMaterial( "Textures/HouseTowelToon.png" ) );
  mm->setSubMaterial( 10, loadTexMaterial( "Textures/TVdotsToon.png" ) );
  mm->setSubMaterial( 11, loadTexMaterial( "Textures/TVplastic.png" ) );
  mm->setSubMaterial( 12, new StandardMaterial() );
  mm->setSubMaterial( 13, loadTexMaterial( "Textures/StoveTopToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 13 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 14, loadTexMaterial( "Textures/PilllowUV.png" ) );
  mm->setSubMaterial( 15, loadTexMaterial( "Textures/BedHeadUVToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 15 ))->setSpecularity( 0.8f );
  mm->setSubMaterial( 16, loadTexMaterial( "Textures/BedUVToon3.png" ) );
  mm->setSubMaterial( 17, loadTexMaterial( "Textures/HouseWood2Toon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial( 17 ))->setSpecularity( 0.5f );
  ((StandardMaterial*)mm->getSubMaterial( 17 ))->setGlossiness( 0.2f );
  mm->setSubMaterial( 18, loadTexMaterial( "Textures/LeatherSquareButtonToonSmall.png" ) );
  mm->setSubMaterial( 19, loadTexMaterial( "Textures/DarkBrushedMetal.png" ) );
  mm->setSubMaterial( 20, loadTexMaterial( "Textures/LeatherSquareToon.png" ) );
  mm->setSubMaterial( 21, loadTexMaterial( "Textures/DarkBrushedMetal.png" ) );
  mm->setSubMaterial( 22, loadTexMaterial( "Textures/LeatherSquareToon.png" ) );
  mm->setSubMaterial( 23, new StandardMaterial() );

  Actor *house = loadActor( "Meshes/House2.pak" );
  house->scale( 10 );
  house->setMaterial( mm );
  scene->addChild( house );

  //Create lights
  light = new SpotLight( Vector3(0,320,-120), Vector3(0.5,-1,0), 70, 0 );
  light->setCastShadows( true );
  light->setDiffuseColor( Vector3(1) );
  scene->addChild( light );

  Light *light2 = new SpotLight( Vector3(-200,400,-120), Vector3(-0.5,-1,0), 70, 0 );
  light2->setDiffuseColor( Vector3(1) );
  light2->setCastShadows( true );
  scene->addChild( light2 );

  Light *light5 = new SpotLight( Vector3(-100,400,-120), Vector3(0,-1,0), 70, 0 );
  light5->setDiffuseColor( Vector3(1) );
  light5->setCastShadows( true );
  scene->addChild( light5 );

  Light *light3 = new SpotLight( Vector3(400,300,-100), Vector3(0,-1,0), 60, 0 );
  light3->setDiffuseColor( Vector3(1) );
  light3->setCastShadows( true );
  scene->addChild( light3 );

  Light *light4 = new SpotLight( Vector3(-100,250,300), Vector3(0,-1,0), 100, 0 );
  light4->setDiffuseColor( Vector3(1) );
  light4->setCastShadows( true );
  scene->addChild( light4 );

  Light *light6 = new SpotLight( Vector3(150,300,200), Vector3(0,-1,0), 60, 0 );
  light6->setDiffuseColor( Vector3(1) );
  light6->setCastShadows( true );
  scene->addChild( light6 );
*/

  //Create floor cube
  StandardMaterial *matBox = new StandardMaterial;
  matBox->setSpecularity( 0.5 );
  matBox->setDiffuseColor( Vector3(1,1,1) );

  TriMesh *cubeMesh = new CubeMesh;
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( matBox );
  cube->setMesh( cubeMesh );
  cube->scale( 300, 10, 300 );
  cube->translate( 0, -100, 0 );
  scene->addChild( cube );

  //Create lights
  light = new SpotLight( Vector3(-200,300,-200), Vector3(), 60, 0 );
  light->setCastShadows( true );
  light->setDiffuseColor( Vector3(1) );
  light->setSpecularColor( Vector3(5) );
  light->lookInto( center );
  scene->addChild( light );

  //Setup 3D camera
  camSky = new Camera3D;
  cam3D = new Camera3D;
  cam3D->setCenter( center );
  cam3D->translate( 0, 0, -300 );
  cam3D->orbitV( Util::DegToRad( 25 ) );
  cam3D->setNearClipPlane( 1.0f );
  cam3D->setFarClipPlane( 3000.0f );

  //Attach to controller
  ctrl = new FpsController;
  ctrl->attachCamera( cam3D );
  ctrl->setMoveSpeed( 200 );

  //Setup 2D overlay
  UI::Stage *stage = new UI::Stage;
  uictrl = new UICtrl();
  window = new UI::Window;

  lblFps = new FpsLabel;
  lblFps->setLoc( Vector2( 0.0f, (Float)resY-20.0f ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  lblFps->setParent( window );

  cam2D = new Camera2D;

  //Setup current scene to render
  sceneRender = scene;
  camRender = cam3D;

  //Setup conversation
  ConvoSpeaker *speaker1 = new ConvoSpeaker;
  speaker1->color = Vector3( 0,1,0 );

  ConvoSpeaker *speaker2 = new ConvoSpeaker;
  speaker2->color = Vector3( 1,0,0 );

  convo = new Convo;

  ConvoSpeach *s1 = new ConvoSpeach;
  s1->speaker = speaker1;
  s1->duration = 2;
  s1->text = "Speach1 Speach1 Speach1";
  convo->next = s1;

  ConvoSpeach *s2 = new ConvoSpeach;
  s2->speaker = speaker2;
  s2->duration = 2;
  s2->text = "Speach2 Speach2 Speach2";
  s1->next = s2;

  ConvoBranch *s2branch = new ConvoBranch;
  s2branch->duration = 100;
  s2->next = s2branch;

  ConvoSpeach *s3 = new ConvoSpeach;
  s3->speaker = speaker2;
  s3->duration = 2;
  s3->text = "Default Default Default";
  s2branch->next = s3;
  s3->next = s1;

  ConvoOpt *s2o1 = new ConvoOpt;
  s2o1->text = "Opt1";
  s2branch->options.pushBack( s2o1 );

  ConvoOpt *s2o2 = new ConvoOpt;
  s2o2->text = "Opt2";
  s2branch->options.pushBack( s2o2 );

  ConvoOpt *s2o3 = new ConvoOpt;
  s2o3->text = "Opt3";
  s2branch->options.pushBack( s2o3 );

  ConvoSpeach *o1 = new ConvoSpeach;
  o1->speaker = speaker2;
  o1->duration = 2;
  o1->text = "Reply1 Reply1 Reply1";
  s2o1->next = o1;
  o1->next = s1;

  ConvoSpeach *o2 = new ConvoSpeach;
  o2->speaker = speaker2;
  o2->duration = 2;
  o2->text = "Reply2 Reply2 Reply2";
  s2o2->next = o2;
  o2->next = s1;

  ConvoSpeach *o3 = new ConvoSpeach;
  o3->speaker = speaker2;
  o3->duration = 2;
  o3->text = "Reply3 Reply3 Reply3";
  s2o3->next = o3;

  convo->begin();

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
