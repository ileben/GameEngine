#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
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

Float animSpeed = 1.0f;

Scene *window = NULL;
FpsLabel *lblFps = NULL;

ByteString data;
TriMesh *mesh = NULL;
Character *character = NULL;
TriMeshActor *triMeshActor = NULL;
SkinMeshActor *skinMeshActor = NULL;
Actor3D *actorRender = NULL;

Light *light = NULL;

Scene3D *sceneSky = NULL;
Camera3D *camSky = NULL;

Scene3D *scene = NULL;
Scene3D *sceneRender = NULL;
AnimController *animCtrl = NULL;

CameraMode::Enum cameraMode;
Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camRender = NULL;
FpsController *ctrl = NULL;

Renderer *renderer = NULL;

bool down3D;
Vector2 lastMouse3D;

int resX = 800;
int resY = 600;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );

DofParams defaultDof;
Float defaultLum;


//////////////////////////////////////////////////////////////
// Menu Hack

static Scene *menuScene = NULL;
static Widget *menuWidget = NULL;
static UICtrl *menuCtrl = NULL;
static bool menuOn = false;

Scene* appMenu ()
{
  //Create scene
  menuScene = new Scene;
  menuScene->setRoot( new Actor );

  //Assign UI controller
  menuCtrl = new UICtrl;
  menuCtrl->bindScene( menuScene );

  return menuScene;
}

void appMenuPage (Widget *menu)
{
  //Remove current page
  if (menuWidget != NULL)
    menuScene->getRoot()->removeChild( menuWidget );

  //Add new page
  menuScene->getRoot()->addChild( menu );
  menuWidget = menu;
}

void appMenuToggle()
{
  menuOn = !menuOn;
}

Widget *menuMain = NULL;
Label *menuOptHouse = NULL;
Label *menuOptCity = NULL;
Label *menuOptExit = NULL;
Label *menuOptUsage = NULL;
Widget *menuUsage = NULL;
Label *menuUsageText = NULL;


void alignMenu (Widget *w)
{
  Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();
  Float spacing = 25.0f;

  UintSize numChildren = w->getChildren().size();
  Float topY = winSize.y * 0.5f - numChildren * spacing * 0.5f;
  Float leftX = winSize.x * 0.5f - 50.0f;

  for (UintSize m=0; m<numChildren; ++m)
  {
    w->getChildren().at( m )->setLoc( leftX, topY + m * spacing );
  }
}

class MenuOpt : public Label
{
public:
  MenuOpt (const CharString &text) {
    setColor( Vector3( 1,1,1 ) );
    setText( text );
  }

  virtual void onMouseEnter (MouseEnterEvent *e) {
    setColor( Vector3( 1,1,0 ) );
  }

  virtual void onMouseLeave (MouseLeaveEvent *e) {
    setColor( Vector3( 1,1,1 ) );
  }
};

class MenuOptUsage : public MenuOpt
{
public:
  MenuOptUsage (const CharString &text)
    : MenuOpt (text) {}

  virtual void onMouseClick (MouseClickEvent *e)
  {
    Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();
    menuUsageText->setLoc( winSize.x * 0.5f - 100.0f, winSize.y * 0.5f - 200 );
    appMenuPage( menuUsage );
  }
};

class MenuOptHouse : public MenuOpt
{
public:
  MenuOptHouse (const CharString &text)
    : MenuOpt (text) {}

  virtual void onMouseClick (MouseClickEvent *e)
  {
    CharString workDir = File("..\\House").getPathName();
    CharString exeFile = File("..\\House\\House.exe").getPathName();
    HINSTANCE inst = ShellExecute( 0, "open", exeFile.buffer(), "", workDir.buffer(), SW_SHOW );
    
    if (reinterpret_cast<int>( inst ) <= 32)
    {
      std::cout << "Failed starting the House application." << std::endl;
      std::cout << "File: " << exeFile.buffer() << std::endl;
      std::cout << "WorkDir: " << workDir.buffer() << std ::endl;
    }
    else
      exit( 0 );
  }
};

class MenuOptCity : public MenuOpt
{
public:
  MenuOptCity (const CharString &text)
    : MenuOpt (text) {}

  virtual void onMouseClick (MouseClickEvent *e)
  {
    CharString workDir = File("..\\City").getPathName();
    CharString exeFile = File("..\\City\\City.exe").getPathName();
    HINSTANCE inst = ShellExecute( 0, "open", exeFile.buffer(), "", workDir.buffer(), SW_SHOW );
    
    if (reinterpret_cast<int>( inst ) <= 32)
    {
      std::cout << "Failed starting the City application." << std::endl;
      std::cout << "File: " << exeFile.buffer() << std::endl;
      std::cout << "WorkDir: " << workDir.buffer() << std ::endl;
    }
    else
      exit( 0 );
  }
};

class MenuOptExit : public MenuOpt
{
public:
  MenuOptExit (const CharString &text)
    : MenuOpt (text) {}

  virtual void onMouseClick (MouseClickEvent *e)
  {
    exit(0);
  }
};

class MenuWidget : public Widget
{
public:
  virtual void draw()
  {
    Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glColor4f( 0,0,0, 0.5f );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glScalef( winSize.x, winSize.y, 1 );

    glBegin( GL_QUADS );
    glVertex2f(0,0);
    glVertex2f(1,0);
    glVertex2f(1,1);
    glVertex2f(0,1);
    glEnd();

    glPopMatrix();

    glDisable( GL_BLEND );
  }
};

//////////////////////////////////////////////////////////////


//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}

void drag3D (int x, int y)
{
  if (!down3D) return;

  Vector2 diff = Vector2( (Float)x,(Float)y ) - lastMouse3D;
  float eyeDist = ( camRender->getEye() - center ).norm();
  lastMouse3D.set( (Float)x, (Float)y );


  if (light != NULL)
  {
    Vector3 side = camRender->getGlobalMatrix( false ).transformVector( camRender->getSide() );
    Vector3 look = Vector::Cross( Vector3(0,1,0), side );
    light->translate( look * diff.y );
    light->translate( side * diff.x );
    light->lookInto( center );
  }

  /*  
  Float angleH = diff.x * (2*PI) / 400;
  Float angleV = diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom =  diff.y * ( eyeDist * 0.01f );

  switch (cameraMode)
  {  
  case CameraMode::Zoom:

    camRender->zoom( zoom );
    break;
    
  case CameraMode::Orbit:
    
    camRender->setCenter( center );
    camRender->orbitH( angleH, true );
    camRender->orbitV( angleV, true );
    break;

  case CameraMode::Pan:

    camRender->panH( panH );
    camRender->panV( panV );
    break;
  }
  */
  postRedisplay();
}

void click3D (int button, int state, int x, int y)
{
  if (state != GLUT_DOWN)
  {
    down3D = false;
    return;
  }

  /*
  int mods = glutGetModifiers();

  if (mods & GLUT_ACTIVE_ALT)
  {
    if (button == GLUT_LEFT_BUTTON)
      cameraMode = CameraMode::Orbit;
    else if (button == GLUT_RIGHT_BUTTON)
      cameraMode = CameraMode::Zoom;
    else if (button == GLUT_MIDDLE_BUTTON)
      cameraMode = CameraMode::Pan;
  }
  else if (mods & GLUT_ACTIVE_CTRL)
  {
    cameraMode = CameraMode::Pan;
  }
  else return;
  */

  lastMouse3D.set( (Float)x, (Float)y );
  down3D = true;
}

void mouseClick (int button, int state, int x, int y)
{
  if (menuOn)
  {
    menuCtrl->mouseClick( button, state, x, y );
    return;
  }

  //if (button == GLUT_LEFT_BUTTON)
    ctrl->mouseClick( button, state, x, y );
  
  //else if (button == GLUT_RIGHT_BUTTON)
    //click3D( button, state, x, y );
}

void mouseMove (int x, int y)
{
  if (menuOn)
  {
    menuCtrl->mouseMove( x, y );
    return;
  }

  //if (button == GLUT_LEFT_BUTTON)
    ctrl->mouseMove( x, y );

  //else if (button == GLUT_RIGHT_BUTTON)
    //drag3D( x, y );
}

void keyDown (unsigned char key, int x, int y)
{
  if (key == 27)
  {
    alignMenu( menuMain );
    appMenuPage( menuMain );
    appMenuToggle();
    return;
  }

  if (menuOn)
    return;

  ctrl->keyDown( key );
  //return;

  switch (key)
  {
  case 9://tab
    camRender->setDofEnabled( !camRender->getDofEnabled() );

    //for (UintSize l=0; l<scene->getLights()->size(); ++l)
      //scene->getLights()->at( l )->setCastShadows( !scene->getLights()->at( l )->getCastShadows() );
    break;

  case 8://backspace
    camRender->setDofEnabled( true );
    camRender->setDofParams( defaultDof );
    renderer->setAvgLuminance( defaultLum );

    //if (skinMeshActor == NULL) return;
    //skinMeshActor->loadPose();
    break;

  case 13://return
    animCtrl->play(0, 0.7f); return;
    /*
    if (skinMeshActor == NULL) return;
    if (!skinMeshActor->getAnimController()->isPlaying())
      skinMeshActor->getAnimController()->play( -1 );
    else skinMeshActor->getAnimController()->toggle();
    break;
*/
  case '+':
    if (skinMeshActor == NULL) return;
    animSpeed += 0.1f;
    skinMeshActor->getAnimController()->setSpeed( animSpeed );
    printf ("Animation speed: %f\n", animSpeed );
    break;

  case '-':
    if (skinMeshActor == NULL) return;
    if (animSpeed > 0.11f) animSpeed -= 0.1f;
    skinMeshActor->getAnimController()->setSpeed( animSpeed );
    printf ("Animation speed: %f\n", animSpeed );
    break;

  case 27:
    //Quit on escape
    //exit(0);
    ;
  }
}

void keyUp (unsigned char key, int x, int y)
{
  ctrl->keyUp( key );
}

void specialKey (int key, int x, int y)
{
  float l;
  float lstep = 0.05f;
  float fstep = 20.0f;
  DofParams dof;

  switch (key)
  {
  case GLUT_KEY_F1:
    l = renderer->getAvgLuminance();
    l = l * 1.1f;
    renderer->setAvgLuminance( l );
    std::cout << "Luminance: " << l << std::endl;
    break;
  case GLUT_KEY_F2:
    l = renderer->getAvgLuminance();
    l = l /= 1.1f;
    renderer->setAvgLuminance( l );
    std::cout << "Luminance: " << l << std::endl;
    break;
  case GLUT_KEY_F3:
    dof = camRender->getDofParams();
    dof.focusCenter = Util::Max( dof.focusCenter-fstep, 0.0f );
    camRender->setDofParams( dof );
    std::cout << "Focus distance: " << dof.focusCenter << std::endl;
    break;
  case GLUT_KEY_F4:
    dof = camRender->getDofParams();
    dof.focusCenter = dof.focusCenter + fstep;
    camRender->setDofParams( dof );
    std::cout << "Focus distance: " << dof.focusCenter << std::endl;
    break;

  }
}

void display ()
{
  Matrix4x4 camWorld = camRender->getMatrix().affineNormalize();
  camWorld.setColumn( 3, Vector4( 0,0,0,1 ) );
  camSky->setMatrix( camWorld );

  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->beginFrame();
  
  //draw model
  renderer->beginDeferred();
  renderer->renderSceneDeferred( sceneSky, camSky );
  renderer->renderSceneDeferred( sceneRender, camRender );
  renderer->endDeferred();
  
  //Frames per second
  renderer->setViewport( 0,0,resX, resY );
  renderer->renderWindow( window, cam2D );

  //Menu
  if (menuOn)
    renderer->renderWindow( menuScene, cam2D );
  
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
    center /= (Float) mesh->getVertexCount();
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
  animCtrl->tick();

  glutPostRedisplay();
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Game Editor" );

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
  idleDraw = true;
}
/*
class VertColorMaterial : public StandardMaterial { public:
  virtual void begin ()  {
    StandardMaterial::begin ();
    glEnable( GL_COLOR_MATERIAL );
  }
};

void SPolyActor::renderMesh (MaterialID matid)
{
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  TexMesh::FaceIter uf( texMesh );
  for (SPolyMesh::FaceIter f( polyMesh ); !f.end(); ++f, ++uf) {
    
    //Check if this face belongs to current material
    if (matid != f->materialID() &&
        matid != GE_ANY_MATERIAL_ID)
      continue;
    
    glBegin( GL_POLYGON );
    
    TexMesh::FaceVertIter uv(*uf);
    for(SPolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
      
      //Adjust the color based on bone weight
      SPolyMesh::Vertex *vert = h->dstVertex();
      glColor3f (1,1,1);
      for (int b=0; b<4; ++b) {
        if (vert->boneIndex[ b ] == boneColorIndex &&
            vert->boneWeight[ b ] > 0.0f)
            glColor3f( 1, 0, 0 ); }
      
      //Vertex normal
      glNormal3fv( (Float*) &h->vertexNormal()->coord );
      
      //UV coordinates
      if (!uv.end()) {
        glTexCoord2f( uv->point.x, uv->point.y ); }
      
      //Vertex coordinate
      glVertex3fv( (Float*) &h->dstVertex()->point );
    }
    
    glEnd();
  }
}*/

bool loadPackage (const CharString &fileName)
{
  //Read the file
  File file( fileName );
  if( !file.open( FileAccess::Read, FileCondition::MustExist )) {
    printf( "Failed opening file '%s'!\n", fileName.buffer() );
    return false;
  }

  //Read signature
  Serializer s;
  ByteString sig = file.read( s.getSignatureSize() );
  if (sig.length() != s.getSignatureSize()) {
    printf( "Missing file signature!\n" );
    file.close();
    return false;
  }

  //Check signature
  if ( ! s.checkSignature( sig.buffer() )) {
    printf( "Invalid file signature!\n" );
    file.close();
    return false;
  }
  
  //Read the rest of the file
  data = file.read( file.getSize() - s.getSignatureSize() );
  file.close();

  //Deserialize
  Object *object = s.deserialize( data.buffer() );

  //Check object class
  if (ClassOf( object ) == ClassName( TriMesh ))
  {
    mesh = (TriMesh*) object;
    mesh->sendToGpu();

    //Report
    printf ("Static mesh: %d verts, %d faces\n",
            mesh->getVertexCount(),
            mesh->getFaceCount());
  }
  else if (ClassOf( object ) == ClassName( Character ))
  {
    character = (Character*) object;
    
    //Animations report
    for (UintSize a=0; a<character->anims.size(); ++a)
      printf ("Animation [%d]: '%s'\n", a,
              character->anims[ a ]->name.buffer());

    //Get animation name
    if (character->anims.size() > 0) {
      char buf[256]; int len=0; UintSize animIndex=0;
      while (len==0 || animIndex >= character->anims.size())
      {
        std::cout << "Pick animation: ";
        std::cin.width( 256 );
        std::cin >> buf;
        CharString str = buf;
        animIndex = str.parseIntegerAt(0, &len);
      }      
      //animName = character->anims[ animIndex ]->name;
    }

    for (UintSize m=0; m<character->meshes.size(); ++m)
      character->meshes[m]->sendToGpu();
  }

  return true;
}

StandardMaterial* loadMaterial (
  const CharString &diffFileName="",
  const CharString &normFileName="" )
{
  Image *imgDiff=NULL, *imgNorm=NULL;
  Texture *texDiff=NULL, *texNorm=NULL;

  if (diffFileName != "")
  {
    imgDiff = new Image;
    imgDiff->readFile( diffFileName, "" );

    texDiff = new Texture;
    texDiff->fromImage( imgDiff );
  }

  if (normFileName != "")
  {
    imgNorm = new Image;
    imgNorm->readFile( normFileName, "" );

    texNorm = new Texture;
    texNorm->fromImage( imgNorm );
  }

  if (texNorm != NULL)
  {
    NormalTexMat *mat = new NormalTexMat;
    if (texDiff != NULL)
      mat->setDiffuseTexture( texDiff );
    mat->setNormalTexture( texNorm );
    mat->setSpecularity( 0.5 );
    return mat;
  }

  if (texDiff != NULL)
  {
    DiffuseTexMat *mat = new DiffuseTexMat;
    mat->setDiffuseTexture( texDiff );
    return mat;
  }

  StandardMaterial *mat = new StandardMaterial;
  return mat;
}

Actor3D* loadActor (const CharString &meshFileName,
                  const CharString &texFileName="",
                  const CharString &normTexFileName="")
{
  mesh = NULL;
  character = NULL;
  actorRender = NULL;

  if (!loadPackage( meshFileName ))
    return NULL;

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
    skinMeshActor->setCharacter( character );
    //meshRender = character->mesh;
    actorRender = skinMeshActor;
  }
/*
  //Center in the scene
  findBounds( meshRender, actorRender->getGlobalMatrix() );
  Vector3 trans = center * -1;
  actorRender->translate( trans.x, trans.y, trans.z );

  //Scale to [100,100,100] range
  Vector3 size = boundsMax - boundsMin;
  Float sizemax = Util::Max( Util::Max( size.x, size.y), size.z );
  Float scale = 100.0f / sizemax;
  actorRender->scale( scale );
  findBounds( meshRender, actorRender->getGlobalMatrix() );
*/
  if (normTexFileName != "")
  {
    //Assign normal-texture material
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
    //Assign diffuse-texture material
    Image *img = new Image;
    img->readFile( texFileName, "jpeg" );

    Texture *tex = new Texture;
    tex->fromImage( img );

    DiffuseTexMat *matTex = new DiffuseTexMat;
    matTex->setDiffuseTexture( tex );
    actorRender->setMaterial( matTex );
  }
  else
  {
    //Assign solid color material
    StandardMaterial *matWhite = new StandardMaterial;
    actorRender->setMaterial( matWhite );
  }

  return actorRender;
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
{/*
  //Usage
  if (argc < 3) {
    printf( "Usage: EXE  SCENE_FILE  MOVE_SPEED" );
    getchar();
    return EXIT_FAILURE;
  }
*/
  //Params
  Float moveSpeed = 300.0f;
  //CharString strMoveSpeed = argv[2];
  //moveSpeed = strMoveSpeed.parseFloat();

  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  renderer->setWindowSize( resX, resY );
  printf( "Kernel loaded\n" );

  //Setup sky scene
  sceneSky = kernel.loadSceneFile( "Export/SkyBox.pak" );
  camSky = (Camera3D*) sceneSky->findFirstActorByClass( ClassName(Camera3D) );


  //Setup 3D scene
  //scene = kernel.loadSceneFile( "Export/TestSkin.pak" );
  //scene = kernel.loadSceneFile( "Export/CityTex.pak" );
  //scene = kernel.loadSceneFile( "Export/CityPlain.pak" );
  //scene = kernel.loadSceneFile( "Export/CityNight.pak" );
  scene = kernel.loadSceneFile( "Export/CityShot2.pak" );
  //scene = kernel.loadSceneFile( argv[1] );
  //scene = kernel.loadSceneFile( "HousePointLights.pak" );
  //scene = kernel.loadSceneFile( "HouseSpotLights.pak" );
  //scene = kernel.loadSceneFile( "CitySpotLights.pak" );
  //scene = kernel.loadSceneFile( "CityPointLights.pak" );
  //scene = kernel.loadSceneFile( "City.pak" );
  //scene = kernel.loadSceneFile( "BossConvo.pak" );
  //scene = kernel.loadSceneFile( "ZacScene.pak" );
  if (scene == NULL) {
    std::cout << "Failed loading scene file!" << std::endl;
    std::getchar();
    return EXIT_FAILURE;
  }

  //Find first skin actor
  skinMeshActor = (SkinMeshActor*) scene->findFirstActorByClass( ClassName(SkinMeshActor) );
  if (skinMeshActor != NULL)
  {
    //Find the name of the first animation
    if (!skinMeshActor->getCharacter()->anims.empty()) {
      Animation *anim = skinMeshActor->getCharacter()->anims.first();
      skinMeshActor->loadAnimation( anim->name );
    }
  }

  //Bind first animation to the controller
  animCtrl = new AnimController;
  if (!scene->animations.empty()) {
    animCtrl->bindAnimation( scene->animations.first() );
    animCtrl->observeAt( 0.0f );
  }
  
  //Bind event observer to the controller
  animCtrl->bindObserver( new EventObserver() );

  //Find first camera in the scene
  cam3D = (Camera3D*) scene->findFirstActorByClass( ClassName(Camera3D) );
  if (cam3D == NULL)
  {
    //Create one if missing
    cam3D = new Camera3D;
    cam3D->setParent( scene->getRoot() );
    cam3D->setNearClipPlane( 1.0f );
    cam3D->setFarClipPlane( 20000.0f );
  }

  DofParams tempDof = cam3D->getDofParams();
  tempDof.focusCenter = tempDof.focusRange;
  cam3D->setDofParams( tempDof );

  defaultDof = cam3D->getDofParams();
  defaultLum = renderer->getAvgLuminance();
  
  //Setup 2D overlay
  Stage *stage = new Stage;
  window = new Scene;
  window->setRoot( new Actor );

  lblFps = new FpsLabel;
  lblFps->setLoc( Vector2( 0.0f, (Float)resY-20 ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  lblFps->setParent( window->getRoot() );

  cam2D = new Camera2D;

  //Menu Hack
  menuMain = new MenuWidget;
  
  menuOptUsage = new MenuOptUsage( "Controls" );
  menuMain->addChild( menuOptUsage );

  menuOptHouse = new MenuOptHouse( "Play House" );
  menuMain->addChild( menuOptHouse );

  menuOptCity = new MenuOptCity( "Play City" );
  menuMain->addChild( menuOptCity );

  menuOptExit = new MenuOptExit( "Exit Game" );
  menuMain->addChild( menuOptExit );

  menuUsage = new MenuWidget;
  menuUsageText = new Label;
  menuUsage->addChild( menuUsageText );

  menuUsageText->setColor( Vector3( 1,1,1 ));
  menuUsageText->setText(
    "Movement:\n"
    "E - Forward\n"
    "S - Left\n"
    "D - Backward\n"
    "F - Right\n"
    "Space - Climb\n"
    "\n"
    "Look:\n"
    "Right click + drag\n"
    "\n"
    "Effects:\n"
    "F1 - lower brightness\n"
    "F2 - higher brightness\n"
    "F3 - closer focus distance (press & hold)\n"
    "F4 - farther focus distance (press & hold)\n"
    "Backspace - reset brightness and focus\n"
    );

  appMenu();

  //Start with Logo scene
  sceneRender = scene;
  camRender = cam3D;

  //Assign controller
  ctrl = new FpsController;
  ctrl->attachCamera( cam3D );
  ctrl->setMoveSpeed( moveSpeed );

  //Tick first time after all setup done
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  kernel.tick( time );

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
