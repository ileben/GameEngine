#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
using namespace GE;

#include <cstdlib>
#include <cstdio>

void applyFK( UintSize frame );

class SPolyActor : public PolyMeshActor { public:
  virtual void renderMesh (MaterialID materialID);
};

namespace CameraMode
{
  enum Enum
  {
    Pan,
    Orbit,
    Zoom
  };
}

ByteString data;
MaxCharacter *character;
SkinMeshActor *skinActor;

Scene *scene;
Light *light;

FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Renderer *renderer;
bool down3D;
Vector2 lastMouse3D;
int boneColorIndex = 0;
UintSize frame = 0;
UintSize numFrames = 0;
float curTime = 0.0f;
float maxTime = 0.0f;

int resY = 512;
int resX = 512;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );
CameraMode::Enum cameraMode;

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
  float eyeDist = ( cam3D.getEye() - center ).norm();
  
  Float angleH = diff.x * (2*PI) / 400;
  Float angleV = diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom = -diff.y * ( eyeDist * 0.01f );
  lastMouse3D.set( (Float)x, (Float)y );
  
  switch (cameraMode)
  {  
  case CameraMode::Zoom:

    cam3D.zoom( zoom );
    break;
    
  case CameraMode::Orbit:
    
    cam3D.setCenter( center );
    cam3D.orbitH( angleH, true );
    cam3D.orbitV( angleV, true );
    break;

  case CameraMode::Pan:

    //cam3D.panH( panH );
    //cam3D.panV( panV );
    Vector3 flatLook = cam3D.getLook();
    flatLook.y = 0.0f; flatLook.normalize();
    Vector3 t = flatLook * -panV + cam3D.getSide() * -panH;
    light->translate( t.x, t.y, t.z );
    light->lookAt( center - light->getMatrix().getColumn(3).xyz(), Vector3(0,1,0) );
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
    if (!skinActor->isAnimationPlaying())
      skinActor->loopAnimation( "MyAnimation", 0.6f );
    else skinActor->pauseAnimation();
    break;

  case '+':
    speed = skinActor->getAnimationSpeed();
    skinActor->setAnimationSpeed( speed + 0.1f );
    printf ("Animation speed: %f\n", skinActor->getAnimationSpeed() );
    break;
    //if (boneColorIndex > 0) boneColorIndex--;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    //if (frame < numFrames-1) ++frame;
    //if (curTime < maxTime) curTime += 0.01f;
    //break;
  case '-':
    speed = skinActor->getAnimationSpeed();
    if (speed > 0.11f) skinActor->setAnimationSpeed( speed - 0.1f );
    printf ("Animation speed: %f\n", skinActor->getAnimationSpeed() );
    break;
    //if (boneColorIndex > 0) boneColorIndex--;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    //if (frame > 0) --frame; 
    //if (curTime > 0.0f) curTime -= 0.01f;
    //break;
  case 27:
    //Quit on escape
    exit(0);
  }
}

void display ()
{
  renderer->renderShadowMap( light, scene );
  renderer->beginFrame();
  
  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( &cam3D );
  
  //draw model
  renderer->beginScene( scene );
  renderer->renderScene();
  renderer->endScene();
  
  //Frames per second
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( &cam2D );
  renderer->renderWidget( &lblFps );
  
  renderer->endFrame();
}

void reshape (int w, int h)
{
  resX = w;
  resY = h;
}

void findCenter ()
{
  int count = 0;
  center.set( 0,0,0 );
  boundsMin.set( 0,0,0 );
  boundsMax.set( 0,0,0 );

  SkinTriMesh *mesh = character->mesh;
  for (UintSize v=0; v<mesh->data.size(); ++v) {
    center += mesh->getVertex( v )->point;
    count++;
  }
  
  center /= (Float)count;
}

void cleanup()
{
}

void animate()
{
  Float time = (Float) glutGet( GLUT_ELAPSED_TIME ) * 0.001f;
  Kernel::GetInstance()->tick( time );
  skinActor->tick( );

  glutPostRedisplay();
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE );

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
}

void loadPackage (String fileName)
{ 
  //Read the file
  File file( fileName );
  if( !file.open( "rb" ))
  {
    printf( "Failed opening file!\n" );
    getchar();
    exit( 1 );
  }
  
  data = file.read( file.getSize() );
  file.close();
  
  //Load character data
  SerializeManager sm;
  character = (MaxCharacter*) sm.load( (void*)data.buffer() );
  
  printf ("Imported %d verts, %d faces, %d animations\n",
          character->mesh->getVertexCount(),
          character->mesh->getFaceCount(),
          character->anims.size());
  
  printf ("Animation name: '%s'\n",
          character->anims.first()->name.buffer());

  numFrames = character->anims.first()->tracksR.first()->keys.size();
  maxTime = character->anims.first()->tracksR.first()->totalTime;

  /*
  //Add vertices to the mesh
  polyMesh = new SPolyMesh;
  ArrayList <SPolyMesh::Vertex*> verts( inMesh->verts.size() );
  for (UintSize v=0; v<inMesh->verts.size(); ++v)
  {
    SPolyMesh::Vertex *vert = (SPolyMesh::Vertex*) polyMesh->addVertex();
    vert->point = inMesh->verts[v].point;
    for (int b=0; b<4; ++b) {
      vert->boneIndex[b] = inMesh->verts[v].boneIndex[b];
      vert->boneWeight[b] = inMesh->verts[v].boneWeight[b]; }
    verts.pushBack (vert);
  }
  
  //Add indexed faces to the mesh
  int nextIndex = 0;
  for (UintSize f=0; f<inMesh->faces.size(); ++f)
  {
    int numCorners = inMesh->faces[f].numCorners;
    HMesh::Vertex **corners = new HMesh::Vertex*[ numCorners ];
    
    for (int c=0; c<numCorners; ++c) {
      Uint32 vertIndex = inMesh->indices[ nextIndex++ ];
      if (vertIndex > inMesh->verts.size()) {
        printf( "Invalid vertex: %d\n", vertIndex );
        vertIndex = 0; }
      corners[c] = verts[ vertIndex ];
    }
    
    SPolyMesh::Face *face = (SPolyMesh::Face*) polyMesh->addFace( corners, numCorners );
    if (face != NULL) face->smoothGroups = inMesh->faces[f].smoothGroups;
    
    delete[] corners;
  }
  */ 
}

void applyFK (UintSize frame)
{
  /*
  int vindex = 0; int nindex = 0;

  //Transform vertices
  for (SPolyMesh::VertIter v(polyMesh); !v.end(); ++v, ++vindex)
  {
    v->point.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      Vector3 &posePoint = polyPosePoints[ vindex ];
      Vector3 skinPoint = skinMats[ v->boneIndex[i] ] * posePoint;
      v->point += skinPoint * v->boneWeight[i];
    }
  }
  
  //Transform normals
  for (SPolyMesh::VertexNormalIter n(polyMesh); !n.end(); ++n, ++nindex)
  {
    n->coord.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      SPolyMesh::Vertex *v = (SPolyMesh::Vertex*) n->vert;
      Vector3 &poseNormal = polyPoseNormals[ nindex ];
      Vector3 skinNormal = skinMats[ v->boneIndex[i] ].transformVector( poseNormal );
      n->coord += skinNormal * v->boneWeight[i];
    }
  } */
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );
  
  //Setup camera
  cam3D.setCenter( center );
  cam3D.translate( 0,0,-400 );
  cam3D.orbitV( Util::DegToRad( 20 ), true );
  cam3D.orbitH( Util::DegToRad( 30 ), true );
  cam3D.setNearClipPlane( 10.0f );
  cam3D.setFarClipPlane( 1000.0f );
  
  Shader *shader = new Shader;
  shader->fromFile( "pixelphong.vert.c", "pixelphong.frag.c" );
  //shader->registerUniform( "sampler", GE_UNIFORM_TEXTURE, 1 );

  //VertColorMaterial mat;
  StandardMaterial mat;
  //PhongMaterial mat;
  mat.setSpecularity( 0.5 );
  //mat.setCullBack( false );
  //mat.setDiffuseColor( Vector3(1,0,0) );
  mat.setShader( shader );

  scene = new Scene;
  
  loadPackage( "bub.pak" );

  skinActor = new SkinMeshActor;
  skinActor->setMaterial( &mat );
  skinActor->setMesh( character );
  scene->addChild( skinActor );
  
  applyFK( 0 );

  TriMesh *cubeMesh = new CubeMesh;
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( &mat );
  cube->setMesh( cubeMesh );
  cube->scale( 300, 10, 300 );
  cube->translate( 0, -70, 0 );
  scene->addChild( cube );

  StandardMaterial axesMat;
  axesMat.setUseLighting( false );
  AxisActor *axes = new AxisActor;
  axes->scale( 100 );
  axes->setMaterial( &axesMat );
  //scene->addChild( axes );

  light = new SpotLight( Vector3(-200,200,-200), Vector3(1,-1,1), 60, 0 );
  scene->addChild( light );
  
  lblFps.setLocation( Vector2( 0.0f, (Float)resY ));
  lblFps.setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  
  //Find model center
  findCenter();
  cam3D.setCenter( center );

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
