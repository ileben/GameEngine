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

Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camRender = NULL;
CameraMode::Enum cameraMode;

Renderer *renderer = NULL;

bool down3D;
Vector2 lastMouse3D;

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
    
    camRender->setCenter( center );
    camRender->orbitH( angleH, true );
    camRender->orbitV( angleV, true );
    break;

  case CameraMode::Pan:

    camRender->panH( panH );
    camRender->panV( panV );
    break;
  }
  
  postRedisplay();
}

void click3D (int button, int state, int x, int y)
{
  if (state != GLUT_DOWN)
  {
    down3D = false;
    return;
  }

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
  {
  case 9://tab
    light->setCastShadows( !light->getCastShadows() );
    break;

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

  case 27:
    //Quit on escape
    exit(0);
  }
}

void specialKey (int key, int x, int y)
{
  float l;
  float lstep = 0.05f;

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
  glutSpecialFunc( specialKey );
  glutMouseFunc( click );
  glutMotionFunc( drag );
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

  if (!loadPackage( meshFileName ))
    return NULL;

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

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );

  //Setup 3D scene
  scene = new Scene;

  if (argc < 4)
  {
    //Get input filename and load it
    CharString fileName;
    do {
      char buf[256];
      std::cout << "Filename: ";
      std::cin.width( 256 );
      std::cin >> buf;
      fileName = buf;
      loadActor( fileName );
    }
    while (actorRender == NULL);
  }
  else
  {
    //Take filename from argument
    CharString fileName = argv[1];

    CharString texFileName = argv[2];
    if (texFileName == "notex")
      texFileName = "";

    animName = argv[3];
    if (animName == "noanim")
      animName = "";

    loadActor( fileName, texFileName );
    if (actorRender == NULL) {
      printf( "Failed loading '%s'\n", argv[1] );
      getchar();
      return 1;
    }
  }

  scene->addChild( actorRender );
  ((StandardMaterial*)actorRender->getMaterial())->setCullBack(false);
  ((StandardMaterial*)actorRender->getMaterial())->setLuminosity(0.2f);
  ((StandardMaterial*)actorRender->getMaterial())->setDiffuseColor(Vector3(.7,.7,.7));
  ((StandardMaterial*)actorRender->getMaterial())->setSpecularity(1.0f);
  ((StandardMaterial*)actorRender->getMaterial())->setGlossiness(0.5f);
  ((StandardMaterial*)actorRender->getMaterial())->setCellShaded( true );
  
/*
  loadActor( "trex.pak", "rex.jpg", "rex_normal.jpg" );
  //((StandardMaterial*)actorRender->getMaterial())->setCullBack(false);
  ((StandardMaterial*)actorRender->getMaterial())->setDiffuseColor(Vector3(0,.5,0));
  ((StandardMaterial*)actorRender->getMaterial())->setSpecularity(0.9f);
  ((StandardMaterial*)actorRender->getMaterial())->setGlossiness(0.05f);
  ((StandardMaterial*)actorRender->getMaterial())->setCellShaded( true );
  actorRender->scale(1,-1,-1);
  actorRender->translate(0,-50,0);
  scene->addChild( actorRender );
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

  //Create axes
  StandardMaterial axesMat;
  axesMat.setUseLighting( false );
  AxisActor *axes = new AxisActor;
  axes->scale( 100 );
  axes->setMaterial( &axesMat );
  //scene->addChild( axes );

  //Create lights
  light = new SpotLight( Vector3(-200,300,-200), Vector3(), 60, 0 );
  //light = new SpotLight( Vector3(-200,150,200), Vector3(), 60, 0 );
  //light = new SpotLight( Vector3(300,300,50), Vector3(), 60, 0 );
  light->setCastShadows( true );
  light->setDiffuseColor( Vector3(1,1,1) );
  light->setSpecularColor( Vector3(5,5,5) );
  light->lookInto( center );
  scene->addChild( light );

  Light *light2 = new SpotLight( Vector3(300,300,50), Vector3(), 60, 0 );
  light2->lookInto( center );
  light2->setDiffuseColor( Vector3(.5,.5,1) );
  scene->addChild( light2);

  Light *light3 = new SpotLight( Vector3(-200,150,200), Vector3(), 60, 0 );
  light3->lookInto( center );
  light3->setDiffuseColor( Vector3(.5,.5,.5) );
  scene->addChild( light3);

  cam3D = new Camera3D;
  cam3D->setCenter( center );
  cam3D->translate( 0, 0, -300 );
  cam3D->orbitV( Util::DegToRad( 25 ) );
  cam3D->setNearClipPlane( 1.0f );
  cam3D->setFarClipPlane( 3000.0f );

  //Setup 2D overlay
  lblFps = new FpsLabel;
  lblFps->setLocation( Vector2( 0.0f, (Float)resY ));
  lblFps->setColor( Vector3( 1.0f, 1.0f, 1.0f ));

  cam2D = new Camera2D;

  //Start with Logo scene
  sceneRender = scene;
  camRender = cam3D;

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
