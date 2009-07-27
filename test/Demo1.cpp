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
MaxCharacter *character = NULL;
TriMeshActor *triMeshActor = NULL;
SkinMeshActor *skinMeshActor = NULL;
Actor *actorRender = NULL;

Light *light = NULL;

Scene *scene = NULL;
Scene *sceneRender = NULL;

FpsController *ctrl = NULL;
Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camRender = NULL;
CameraMode::Enum cameraMode;

Renderer *renderer = NULL;

int resX = 800;
int resY = 600;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );

//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}

void click (int button, int state, int x, int y)
{
  ctrl->mouseClick( button, state, x, y );
}

void drag (int x, int y)
{
  ctrl->mouseMove( x, y );
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

void display ()
{
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
  renderer->renderWidget( lblFps );
  
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
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );
  
  if (skinMeshActor != NULL)
    skinMeshActor->tick();

  ctrl->tick();

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
  if (cls == Class(TriMesh) || cls == Class(TanTriMesh))
  {
    mesh = (TriMesh*) object;
    mesh->sendToGpu();

    //Report
    printf ("Static mesh: %d verts, %d faces\n",
            mesh->getVertexCount(),
            mesh->getFaceCount());
  }
  else if (cls == Class(MaxCharacter))
  {
    character = (MaxCharacter*) object;
    
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
    if (ClassOf(mesh) == Class(TriMesh))
    {
      triMeshActor = new TriMeshActor;
      triMeshActor->setMesh( mesh );
      meshRender = mesh;
      actorRender = triMeshActor;
    }
    else if (ClassOf(mesh) == Class(TanTriMesh))
    {
      triMeshActor = new TanTriMeshActor;
      triMeshActor->setMesh( mesh );
      meshRender = mesh;
      actorRender = triMeshActor;
    }
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
  mat->setLuminosity( 0.1f );
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
  renderer->setDofParams( 50, 50, 150, 150 );

  //Setup 3D scene
  scene = new Scene;

  MultiMaterial *mm = new MultiMaterial;
  mm->setNumSubMaterials( 27 );
  mm->setSubMaterial( 0, loadTexMaterial( "Textures/BrushedMetal.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(0))->setSpecularity( 0.7f );
  mm->setSubMaterial( 1, loadTexMaterial( "Textures/MediumWoodSquareToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(1))->setSpecularity( 0.7f );
  mm->setSubMaterial( 2, loadTexMaterial( "Textures/LobbyCarpetToon.png" ) );
  mm->setSubMaterial( 3, loadTexMaterial( "Textures/WhiteMarbleTiles4x4Toon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(3))->setSpecularity( 0.7f );
  mm->setSubMaterial( 4, loadTexMaterial( "Textures/GreenWhiteTilesToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(4))->setSpecularity( 0.7f );
  mm->setSubMaterial( 5, loadTexMaterial( "Textures/BlackMarbleSkirtToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(5))->setSpecularity( 0.7f );
  mm->setSubMaterial( 6, loadTexMaterial( "Textures/WoodenSkirtToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(6))->setSpecularity( 0.4f );
  mm->setSubMaterial( 7, loadTexMaterial( "Textures/LobbyRoofToon.png" ) );
  mm->setSubMaterial( 8, new StandardMaterial() );
  mm->setSubMaterial( 9, loadTexMaterial( "Textures/WhiteMarbleTiles4x4Toon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(9))->setSpecularity( 0.7f );
  mm->setSubMaterial( 10, loadTexMaterial( "Textures/BrushedMetal.png" ) );
  mm->setSubMaterial( 11, loadTexMaterial( "Textures/ElevatorLight.png" ) );
  mm->setSubMaterial( 12, loadTexMaterial( "Textures/SmallChairUV.png" ) );
  mm->setSubMaterial( 13, loadTexMaterial( "Textures/BigCouchBlue.png" ) );
  mm->setSubMaterial( 14, loadTexMaterial( "Textures/HouseWood5Toon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(14))->setSpecularity( 0.4f );
  mm->setSubMaterial( 15, loadTexMaterial( "Textures/RedWoodToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(15))->setSpecularity( 0.4f );
  mm->setSubMaterial( 16, loadTexMaterial( "Textures/MaxOfficeFloor.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(16))->setSpecularity( 0.7f );
  ((StandardMaterial*)mm->getSubMaterial(16))->setGlossiness( 0.1f );
  mm->setSubMaterial( 17, loadTexMaterial( "Textures/MarbleBenchUV.png" ) );
  mm->setSubMaterial( 18, loadTexMaterial( "Textures/Grass.png" ) );
  mm->setSubMaterial( 19, loadTexMaterial( "Textures/LobbyPlant512.png" ) );
  mm->setSubMaterial( 20, loadTexMaterial( "Textures/PalmLeaf.png" ) );
  mm->setSubMaterial( 21, loadTexMaterial( "Textures/Leaf.png" ) );
  mm->setSubMaterial( 22, loadTexMaterial( "Textures/GreenPot.png" ) );
  mm->setSubMaterial( 23, new StandardMaterial() );
  mm->setSubMaterial( 24, loadTexMaterial( "Textures/FrostedGlass.png" ) );
  mm->setSubMaterial( 25, loadTexMaterial( "Textures/DarkBrushedMetal.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(25))->setSpecularity( 0.7f );
  mm->setSubMaterial( 26, loadTexMaterial( "Textures/RedWoodToon.png" ) );
  ((StandardMaterial*)mm->getSubMaterial(26))->setSpecularity( 0.4f );
  mm->setSubMaterial( 27, new StandardMaterial() );

  Actor *lobby = loadActor( "Meshes/Lobby.pak" );
  lobby->scale( 10 );
  lobby->setMaterial( mm );
  scene->addChild( lobby );

  loadActor( "Meshes/Ash.pak", "Textures/CharacterAsh.jpg" );
  actorRender->scale( .8 );
  actorRender->translate( 50, 28, -100 );
  scene->addChild( actorRender );
  ((StandardMaterial*)actorRender->getMaterial())->setCullBack( false );
  ((StandardMaterial*)actorRender->getMaterial())->setLuminosity(0.1f);
  ((StandardMaterial*)actorRender->getMaterial())->setDiffuseColor(Vector3(1));
  ((StandardMaterial*)actorRender->getMaterial())->setGlossiness(0.5f);
  ((StandardMaterial*)actorRender->getMaterial())->setCellShaded( true );

  //Create lights
  light = new SpotLight( Vector3(-300,300,-50), Vector3(0,-1,0), 100, 0 );
  light->setCastShadows( true );
  light->setDiffuseColor( Vector3(1) );
  scene->addChild( light );

  Light *light2 = new SpotLight( Vector3(-50,300,50), Vector3(.5,-1,-.5), 60, 0 );
  light2->setDiffuseColor( Vector3(.8) );
  light2->setSpecularColor( Vector3(5) );
  light2->setCastShadows( true );
  scene->addChild( light2 );

  Light *light5 = new SpotLight( Vector3(200,300,-200), Vector3(-.5,-1,.5), 60, 0 );
  light5->setDiffuseColor( Vector3(.8) );
  light5->setSpecularColor( Vector3(5) );
  light5->setCastShadows( true );
  scene->addChild( light5 );

  Light *light4 = new SpotLight( Vector3(400,300,200), Vector3(0,-1,0), 100, 0 );
  light4->setDiffuseColor( Vector3(1) );
  light4->setCastShadows( true );
  scene->addChild( light4 );

  Light *light3 = new SpotLight( Vector3(400,300,-200), Vector3(0,-1,0), 100, 0 );
  light3->setDiffuseColor( Vector3(1) );
  light3->setCastShadows( true );
  scene->addChild( light3 );

  //Setup 3D camera
  cam3D = new Camera3D;
  cam3D->setCenter( center );
  cam3D->orbitH( Util::DegToRad( -25 ) );
  cam3D->orbitV( Util::DegToRad( 10 ) );
  cam3D->translate( 100, 40, -200 );
  cam3D->setNearClipPlane( 1.0f );
  cam3D->setFarClipPlane( 3000.0f );

  //Attach to controller
  ctrl = new FpsController;
  ctrl->attachCamera( cam3D );
  ctrl->setMoveSpeed( 200 );

  //Setup 2D overlay
  lblFps = new FpsLabel;
  lblFps->setLocation( Vector2( 0.0f, (Float)resY ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  cam2D = new Camera2D;

  //Setup current scene to render
  sceneRender = scene;
  camRender = cam3D;

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
