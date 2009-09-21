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

CharString animName;
Float animSpeed = 1.0f;

FpsLabel *lblFps = NULL;

ByteString data;
TriMesh *mesh = NULL;
Character *character = NULL;
TriMeshActor *triMeshActor = NULL;
SkinMeshActor *skinMeshActor = NULL;
Actor *actorRender = NULL;

Light *light = NULL;
ArrayList<Light*> lights;
int lightCols = 2;
int lightRows = 2;
Label *lblLights = NULL;

Scene *sceneSky = NULL;
Scene *scene = NULL;
Scene *sceneRender = NULL;

Camera3D *camSky = NULL;
Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camRender = NULL;
CameraMode::Enum cameraMode = CameraMode::Orbit;

Renderer *renderer = NULL;

bool down3D;
Vector2 lastMouse3D;

int resX = 800;
int resY = 600;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );

float moveSpeed = 15.0f;
int moveDir = 0;
float strafeSpeed = 15.0f;
int strafeDir = 0;
float climbSpeed = 15.0f;
int climbDir = 0;

bool lastTimeInit = false;
Float lastTime = 0.0f;

//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}

void updateLights()
{
  //Clear old
  for (int l=0; l<lights.size(); ++l)
  {
    Light *lt = lights[l];
    scene->removeChild( lt );
    delete lt;
  }

  lights.clear();

  //Make new
  float xStep = 100.0f / lightCols;
  float yStep = 100.0f / lightRows;

  for (int x=0; x<lightCols; ++x)
  {
    for (int y=0; y<lightRows; ++y)
    {
      light = new SpotLight( Vector3(-30 + x*xStep, 20, -30 + y*yStep ), Vector3(0,-1,0), 30, 0 );
      light->lookAt( Vector3(0,-1,0), Vector3(1,0,0) );
      light->setDiffuseColor( Vector3(1,1,1) );
      scene->addChild( light );
      lights.pushBack( light );
    }
  }

  lblLights->setText( CharString::Format( "Num lights: %d\n", lightRows * lightCols ));
}

void updateSkyCamera()
{
  Matrix4x4 mSky = camRender->getMatrix();
  mSky.setColumn( 3, camSky->getMatrix().getColumn(3) );
  camSky->setMatrix( mSky );
}

void drag3D (int x, int y)
{  
  Vector2 diff = Vector2( (Float)x,(Float)y ) - lastMouse3D;
  float eyeDist = ( camRender->getEye() - center ).norm();
  
  Float angleH = diff.x * (2*PI) / 400;
  Float angleV = diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom =  diff.y * ( eyeDist * 0.01f );
  lastMouse3D.set( (Float)x, (Float)y );
  
  switch (cameraMode)
  {  
  case CameraMode::Zoom:

    camRender->zoom( zoom );
    break;
    
  case CameraMode::Orbit:
    
    //camRender->setCenter( center );
    camRender->setCenter( camRender->getEye() );
    camRender->orbitH( angleH, true );
    camRender->orbitV( angleV, true );
    break;

  case CameraMode::Pan:

    camRender->panH( panH );
    camRender->panV( panV );
    break;
  }
  
  updateSkyCamera();
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

void click (int button, int state, int x, int y)
{
  click3D( button, state, x, y );
}

void drag (int x, int y)
{
  if (down3D)
    drag3D( x, y );
}

void keyboard (unsigned char key, int x, int y)
{
  switch (key)
  {/*
  case 8://backspace
    if (skinMeshActor == NULL) return;
    skinMeshActor->loadPose();
    break;

  case 13://return
    if (skinMeshActor == NULL) return;
    skinMeshActor->playAnimation( animName, animSpeed );
    break;

  case ' ':
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
    */
  case '+':
    lightCols++;
    lightRows++;
    updateLights();
    break;
  case '-':
    if (lightCols <= 2) break;
    lightCols--;
    lightRows--;
    updateLights();
    break;
  case 9://tab
    light->setCastShadows( !light->getCastShadows() );
    break;
  case 'e':
    moveDir = 1;
    break;
  case 'd':
    moveDir = -1;
    break;
  case 'f':
    strafeDir = 1;
    break;
  case 's':
    strafeDir = -1;
    break;
  case ' ':
    climbDir = 1;
    break;

  case 27:
    //Quit on escape
    exit(0);
  }
}

void keyboardUp (unsigned char key, int x, int y)
{
  switch (key)
  {
  case 'e':
    if (moveDir == 1) moveDir = 0;
    break;
  case 'd':
    if (moveDir == -1) moveDir = 0;
    break;
  case 'f':
    if (strafeDir == 1) strafeDir = 0;
    break;
  case 's':
    if (strafeDir == -1) strafeDir = 0;
    break;
  case ' ':
    if (climbDir == 1) climbDir = 0;
    break;
  }
}

void display ()
{
  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->beginFrame();
  
  //draw model
  renderer->beginDeferred();
  renderer->setCamera( camSky );
  renderer->renderSceneDeferred( sceneSky );
  renderer->setCamera( camRender );
  renderer->renderSceneDeferred( sceneRender );
  renderer->endDeferred();
  
  //Frames per second
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( cam2D );
  renderer->renderWidget( lblFps );
  renderer->renderWidget( lblLights );
  
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
  if (mesh->getVertexCount() > 0)
    center = boundsMin = boundsMax = xform * mesh->getVertex(0)->point;

  for (UintSize v=1; v<mesh->getVertexCount(); ++v)
  {
    Vector3 point = xform * mesh->getVertex( v )->point;
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
  //Kernel time
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );
  
  //Animation
  if (skinMeshActor != NULL)
    skinMeshActor->tick();

  //Moving
  if (!lastTimeInit) {
    lastTime = time;
    lastTimeInit = true;
  }

  Float interval = time - lastTime;
  lastTime = time;

  if (moveDir != 0) {
    Vector3 look = camRender->getLook();
    camRender->translate( look * moveSpeed * (Float)moveDir * interval );
  }

  if (strafeDir != 0) {
    Vector3 side = camRender->getSide();
    camRender->translate( side * strafeSpeed * (Float)strafeDir * interval );
  }

  if (climbSpeed != 0) {
    Vector3 up = Vector3(0,1,0);
    camRender->translate( up * climbSpeed * (Float)climbDir * interval );
  }

  glutPostRedisplay();
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Game Editor" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutKeyboardUpFunc( keyboardUp );
  glutMouseFunc( click );
  glutMotionFunc( drag );
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

Actor* loadActor (const CharString &meshFileName, const CharString &texFileName="")
{
  mesh = NULL;
  character = NULL;

  if (!loadPackage( meshFileName ))
    return NULL;

  //Create actor
  TriMesh *meshRender = NULL;
  
  if (mesh != NULL)
  {
    triMeshActor = new TriMeshActor;
    triMeshActor->setMesh( mesh );
    actorRender = triMeshActor;
    meshRender = mesh;
  }

  if (character != NULL)
  {
    skinMeshActor = new SkinMeshActor;
    skinMeshActor->setMesh( character );
    actorRender = skinMeshActor;
    //meshRender = character->mesh;
  }
/*
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
*/
  if (texFileName == "")
  {
    //Assign solid color material
    StandardMaterial *matWhite = new StandardMaterial;
    matWhite->setAmbientColor( matWhite->getDiffuseColor() * .8f );
    actorRender->setMaterial( matWhite );
  }
  else
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

  return actorRender;
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );

  //Setup skybox scene
  sceneSky = new Scene;

  Actor *sky = loadActor( "Sky.pak", "SkyBoxMoreBlue.jpg" );
  ((StandardMaterial*)sky->getMaterial())->setLuminosity( 1 );

  sceneSky->addChild( sky );
  camSky = new Camera3D;

  //Setup 3D scene
  scene = new Scene;

  Actor *city = loadActor( "CityDemo.pak" );
  ((StandardMaterial*)city->getMaterial())->setDiffuseColor( Vector3(.5,.6,.9) );
  scene->addChild( city );

  Actor *house = loadActor( "CityDemoHouse.pak", "House01_Texture.jpg" );
  house->scale( 0.02f );
  house->rotate( Vector3(0,1,0), Util::DegToRad(-90) );
  house->translate( 1,-1.4,-18.6);
  scene->addChild( house );

/*
  Actor *bridge = loadActor( "Bridge.pak" );
  bridge->scale( 10 );
  ((StandardMaterial*)bridge->getMaterial())->setLuminosity( .11f );
  scene->addChild( bridge );

  Actor *house1 = loadActor( "House01_Unwrapped.pak", "House01_Texture.jpg" );
  house1->scale( .5f );
  house1->translate( 0,0,-100 );
  scene->addChild( house1 );

  Actor *news1 = loadActor( "Newspaper1.pak" );
  news1->scale( .5f );
  news1->translate( 50,0,-150 );
  scene->addChild( news1 );

  Actor *news2 = loadActor( "Newspaper2.pak" );
  news2->scale( .5f );
  news2->translate( 100,0,-150 );
  scene->addChild( news2 );

  Actor *verasign1 = loadActor( "VeraSign.pak" );
  verasign1->scale( .5f );
  verasign1->translate( 0,0,-150 );
  scene->addChild( verasign1 );

  Actor *verasign2 = loadActor( "VeraSignCastle.pak" );
  verasign2->scale( .5f );
  verasign2->translate( 0,0,-200 );
  scene->addChild( verasign2 );

  //Actor *charActor = loadActor( "mayaexport3.pak" );
  Actor *charActor = loadActor( "Ash.pak" );

  //Clone the actor multiple times
  for (int x=0; x<4; ++x)
  {
    for (int z=0; z<4; ++z)
    {
      SkinMeshActor *a = new SkinMeshActor;
      a->setMesh( character );
      //TriMeshActor *a = new TriMeshActor;
      //a->setMesh( mesh );
      a->setMatrix( charActor->getMatrix() );
      a->translate( -200 + x * 100, 0, -200 + z * 100 );
      a->setMaterial( charActor->getMaterial() );
      //a->setMaterial( matTex );
      scene->addChild( a );
    }
  }
  */

  //Create lights
  /*
  light = new SpotLight( Vector3(-100,200,80), Vector3(), 60, 0 );
  light->setCastShadows( true );
  light->lookInto( center );
  light->setDiffuseColor( Vector3(1,1,1) );
  scene->addChild( light );*/
/*
  Light *light2 = new SpotLight( Vector3(100,200,-80), Vector3(), 60, 0 );
  light2->lookInto( center );
  light2->setDiffuseColor( Vector3(.2,.2,.2) );
  scene->addChild( light2 );
*/

  cam3D = new Camera3D;
  cam3D->setCenter( center );
  cam3D->orbitH( Util::DegToRad( -30 ), false );
  cam3D->orbitV( Util::DegToRad( 30 ), false );
  cam3D->translate( 40, 40, -60 );
  cam3D->setNearClipPlane( 0.1f );
  cam3D->setFarClipPlane( 3000.0f );

  //Setup 2D overlay
  lblFps = new FpsLabel;
  lblFps->setLocation( Vector2( 0.0f, (Float)resY ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));

  lblLights = new Label;
  lblLights->setLocation( Vector2( 0.0f, (Float)resY - 30 ));
  lblLights->setColor( Vector3( 1.0f, 1.0f, 1.0f ));

  cam2D = new Camera2D;

  //Start with Logo scene
  sceneRender = scene;
  camRender = cam3D;
  updateSkyCamera();
  updateLights();

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
