#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
using namespace GE;

#include <cstdlib>
#include <cstdio>

namespace CameraMode
{
  enum Enum
  {
    Pan,
    Orbit,
    Zoom
  };
}

FpsLabel *lblFps = NULL;

ByteString data;
TriMesh *mesh = NULL;
MaxCharacter *character = NULL;
TriMeshActor *triMeshActor = NULL;
SkinMeshActor *skinMeshActor = NULL;
Actor *actorRender = NULL;

Scene *scene = NULL;
Scene *sceneRender = NULL;

Camera2D *cam2D = NULL;
Camera3D *cam3D = NULL;
Camera3D *camRender = NULL;
CameraMode::Enum cameraMode;

Renderer *renderer = NULL;

bool down3D;
Vector2 lastMouse3D;
UintSize frame = 0;
UintSize numFrames = 0;

int resY = 512;
int resX = 512;
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
  //printf ("Eye1:"); printVector (cam3D.getEye());
  //printf ("Look:"); printVector (cam3D.getLook());
  //printf ("Side:"); printVector (cam3D.getSide());
  //printf ("\n");
  
  //printf ("------------------------\n");
  //printMatrix (cam3D.getMatrix());
  
  Vector2 diff = Vector2( (Float)x,(Float)y ) - lastMouse3D;
  float eyeDist = ( camRender->getEye() - center ).norm();
  
  Float angleH = diff.x * (2*PI) / 400;
  Float angleV = diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom = -diff.y * ( eyeDist * 0.01f );
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
  
  if (mods & GLUT_ACTIVE_SHIFT)
  {
    cameraMode = CameraMode::Orbit;
  }
  else if ((mods & GLUT_ACTIVE_CTRL) ||
           button == GLUT_RIGHT_BUTTON)
  {
    cameraMode = CameraMode::Zoom;
  }
  else
  {
    cameraMode = CameraMode::Pan;
  }
  
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
  Float speed;

  switch (key)
  {
  case ' ':
    if (!skinMeshActor->isAnimationPlaying())
      skinMeshActor->loopAnimation( "MyAnimation" );
    else skinMeshActor->pauseAnimation();
    break;

  case '+':
    speed = skinMeshActor->getAnimationSpeed();
    skinMeshActor->setAnimationSpeed( speed + 0.1f );
    printf ("Animation speed: %f\n", skinMeshActor->getAnimationSpeed() );
    break;

  case '-':
    speed = skinMeshActor->getAnimationSpeed();
    if (speed > 0.11f) skinMeshActor->setAnimationSpeed( speed - 0.1f );
    printf ("Animation speed: %f\n", skinMeshActor->getAnimationSpeed() );
    break;

  case 27:
    //Quit on escape
    exit(0);
  }
}

void display ()
{
  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( camRender );
  renderer->beginFrame();
  
  //draw model
  //renderer->renderScene( sceneRender );
  renderer->renderSceneDeferred( sceneRender );
  
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
  glutCreateWindow( "Test Skin" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutMouseFunc( click );
  glutMotionFunc( drag );
  glutIdleFunc( animate );
  idleDraw = true;
}

class VertColorMaterial : public StandardMaterial { public:
  virtual void begin ()  {
    StandardMaterial::begin ();
    glEnable( GL_COLOR_MATERIAL );
  }
};
/*
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

void loadCharacter (String fileName)
{ 
  //Read the file
  File file( fileName );
  if( !file.open( "rb" ))
  {
    printf( "Failed opening file '%s'!\n", fileName.toCSTR().buffer() );
    getchar();
    exit( 1 );
  }
  
  data = file.read( file.getSize() );
  file.close();
  
  //Load character data
  SerializeManager sm;
  character = (MaxCharacter*) sm.load( (void*)data.buffer() );

  void *data; UintSize size;
  SerializeManager sm2;
  sm2.save( character, &data, &size);
  
  printf ("Character: %d verts, %d faces, %d animations\n",
          character->mesh->getVertexCount(),
          character->mesh->getFaceCount(),
          character->anims.size());
  
  for (UintSize a=0; a<character->anims.size(); ++a)
    printf ("Animation name: '%s'\n",
            character->anims[ a ]->name.buffer());

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

void loadMesh (const CharString &fileName)
{
  //Read the file
  File file( fileName );
  if( !file.open( "rb" ))
  {
    printf( "Failed opening file '%s'!\n", fileName.buffer() );
    getchar();
    exit( 1 );
  }
  
  data = file.read( file.getSize() );
  file.close();
  
  //Load mesh data
  SerializeManager sm;
  mesh = (TriMesh*) sm.load( (void*)data.buffer() );
  mesh->sendToGpu();

  printf ("Mesh: %d verts, %d faces\n",
          mesh->getVertexCount(),
          mesh->getFaceCount());
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );

  //Initialize materials
  Shader *shader = new Shader;
  shader->fromFile( "shadevert_geom_gbuf.c", "shadefrag_geom_gbuf.c" );

  Shader *skinShader = new Shader;
  skinShader->registerVertexAttrib( "boneIndex" );
  skinShader->registerVertexAttrib( "boneWeight" );
  skinShader->registerUniform( "skinMatrix", GE_UNIFORM_MATRIX, 1 );
  skinShader->fromFile( "shadevert_geom_skin_gbuf.c", "shadefrag_geom_gbuf.c" );

  StandardMaterial *matWhite = new StandardMaterial;
  matWhite->setSpecularity( 0.5 );
  matWhite->setAmbientColor( matWhite->getDiffuseColor() * .8f );
  matWhite->setShader( shader );

  StandardMaterial *matWhiteSkin = new StandardMaterial;
  matWhiteSkin->setSpecularity( 0.5 );
  matWhiteSkin->setShader( skinShader );

  //Setup 3D scene
  scene = new Scene;

  //Load model
  //loadMesh( "mayatest.pak" );
  loadCharacter( "mayatest.pak" );
  TriMesh *meshRender = NULL;
  /*
  character->mesh->sendToGpu();
  triMeshActor = new TriMeshActor;
  triMeshActor->setMesh( character->mesh );
  triMeshActor->setMaterial( matWhite );
  actorRender = triMeshActor;
  meshRender = character->mesh;*/
  
  if (mesh != NULL)
  {
    triMeshActor = new TriMeshActor;
    triMeshActor->setMesh( mesh );
    triMeshActor->setMaterial( matWhite );
    actorRender = triMeshActor;
    meshRender = mesh;
  }

  if (character != NULL)
  {
    skinMeshActor = new SkinMeshActor;
    skinMeshActor->setMesh( character );
    skinMeshActor->setMaterial( matWhiteSkin );
    actorRender = skinMeshActor;
    meshRender = character->mesh;
  }

  //Add to scene and find bounding box
  scene->addChild( actorRender );
  findBounds( meshRender, actorRender->getWorldMatrix() );
  
  //Scale to [100,100] range and center
  Vector3 size = boundsMax - boundsMin;
  Float sizemax = Util::Max( Util::Max( size.x, size.y), size.z );
  Float scale = 100.0f / sizemax;
  actorRender->translate( -center.x, -center.y, -center.z );
  actorRender->scale( scale );
  findBounds( meshRender, actorRender->getWorldMatrix() );
  
  //Create floor cube
  TriMesh *cubeMesh = new CubeMesh;
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( matWhite );
  cube->setMesh( cubeMesh );
  cube->scale( 300, 10, 300 );
  cube->translate( 0, -100, 0 );
  scene->addChild( cube );

  //Create axes
  StandardMaterial axesMat;
  axesMat.setUseLighting( false );
  AxisActor *axes = new AxisActor;
  axes->scale( 100 );
  axes->setMaterial( matWhite );
  //scene->addChild( axes );

  //Create lights
  Light *light = new SpotLight( Vector3(-200,300,-200), Vector3(1,-1,1), 60, 0 );
  light->setCastShadows( true );
  light->lookInto( center );
  light->setDiffuseColor( Vector3(1,.8,.2) );
  scene->addChild( light );

  Light *light2 = new SpotLight( Vector3(200,300,200), Vector3(-1,-1,-1), 60, 0 );
  light2->lookInto( center );
  light2->setDiffuseColor( Vector3(.5,.5,1) );
  scene->addChild( light2);

  cam3D = new Camera3D;
  cam3D->setCenter( center );
  cam3D->translate( 0,0,-300 );
  cam3D->orbitV( Util::DegToRad( 20 ), true );
  cam3D->orbitH( Util::DegToRad( 30 ), true );
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
