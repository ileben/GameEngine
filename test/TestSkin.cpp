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
TriMesh *mshLogo;
TriMeshActor *actLogo;
SkinMeshActor *skinActor;
SkinMeshActor *skinActor2;

Scene *scene;
Scene *sceneLogo;
Scene *sceneRender = NULL;
Light *light;
Light *lightLogo;
Light *lightRender;

FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Camera3D *camLogo;
Camera3D *camRender = NULL;
Renderer *renderer = NULL;
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

    //camRender->panH( panH );
    //camRender->panV( panV );
    Vector3 flatLook = camRender->getLook();
    flatLook.y = 0.0f; flatLook.normalize();
    Vector3 t = flatLook * -panV + camRender->getSide() * -panH;
    lightRender->translate( t.x, t.y, t.z );
    lightRender->lookAt( center - lightRender->getMatrix().getColumn(3).xyz(), Vector3(0,1,0) );
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
    if (sceneRender == sceneLogo) {
      sceneRender = scene;
      lightRender = light;
      camRender = &cam3D;
      break;
    }

    if (!skinActor->isAnimationPlaying())
      skinActor->loopAnimation( "MyAnimation", 0.6f );
    else skinActor->pauseAnimation();
    break;

  case 13://return
    skinActor2->playAnimation( "MyAnimation", 0.8f );
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
  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( camRender );
  renderer->beginFrame();
  
  //draw model
  //renderer->renderScene( sceneRender );
  renderer->renderSceneDeferred( sceneRender );
  
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

  if (renderer != NULL)
    renderer->setWindowSize( w,h );
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
  skinActor->tick();
  skinActor2->tick();

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
    printf( "Failed opening file '%s'!\n", fileName.toCSTR().buffer() );
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

void loadLogo (String fileName)
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
  mshLogo = (TriMesh*) sm.load( (void*)data.buffer() );

  printf ("Logo: %d verts, %d faces\n",
          mshLogo->getVertexCount(),
          mshLogo->getFaceCount());
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
  
  Shader *shader = new Shader;
  //shader->fromFile( "pixelphong.vert.c", "pixelphong.frag.c" );
  shader->fromFile( "deferred_geometry.vert.c", "deferred_geometry.frag.c" );

  //VertColorMaterial mat;
  StandardMaterial matWhite;
  //PhongMaterial mat;
  matWhite.setSpecularity( 0.5 );
  //mat.setCullBack( false );
  //mat.setDiffuseColor( Vector3(1,0,0) );
  matWhite.setShader( shader );

  StandardMaterial matRed;
  matRed.setSpecularity( 0.5 );
  matRed.setDiffuseColor( Vector3(1,0,0) );
  matRed.setAmbientColor( matRed.getDiffuseColor() * .8f );
  matRed.setShader( shader );

  StandardMaterial matWhiteLogo;
  matWhiteLogo.setSpecularity( 0.5 );
  matWhiteLogo.setAmbientColor( matWhiteLogo.getDiffuseColor() * .8f );
  matWhiteLogo.setShader( shader );

  StandardMaterial matBlue;
  matBlue.setSpecularity( 0.5 );
  matBlue.setDiffuseColor( Vector3(0,0.5,1) );
  matBlue.setShader( shader );

  StandardMaterial matGreen;
  matGreen.setSpecularity( 0.5 );
  matGreen.setDiffuseColor( Vector3(0,0.6,0) );
  matGreen.setShader( shader );

  StandardMaterial matYellow;
  matYellow.setSpecularity( 0.5 );
  matYellow.setDiffuseColor( Vector3(1,1,0.7) );
  matYellow.setShader( shader );

  StandardMaterial matBlack;
  matBlack.setSpecularity( 0.5 );
  matBlack.setDiffuseColor( Vector3(0.2,0.2,0.2) );
  matBlack.setShader( shader );

  MultiMaterial mm;
  mm.setNumSubMaterials( 5 );
  mm.setSubMaterial( 0, &matBlue );
  mm.setSubMaterial( 1, &matYellow );
  mm.setSubMaterial( 2, &matWhite );
  mm.setSubMaterial( 3, &matBlack );
  mm.setSubMaterial( 4, &matBlue );

  MultiMaterial mm2;
  mm2.setNumSubMaterials( 5 );
  mm2.setSubMaterial( 0, &matGreen );
  mm2.setSubMaterial( 1, &matYellow );
  mm2.setSubMaterial( 2, &matWhite );
  mm2.setSubMaterial( 3, &matBlack );
  mm2.setSubMaterial( 4, &matGreen );

  MultiMaterial mmLogo;
  mmLogo.setNumSubMaterials( 2 );
  mmLogo.setSubMaterial( 0, &matWhiteLogo );
  mmLogo.setSubMaterial( 1, &matRed );

  //Setup Logo scene

  sceneLogo = new Scene;

  loadLogo( "logo.pak" );
  actLogo = new TriMeshActor;
  actLogo->setMesh( mshLogo );
  actLogo->translate( 200, 0, 0 );
  actLogo->setMaterial( &mmLogo );
  sceneLogo->addChild( actLogo );

  camLogo = new Camera3D;
  camLogo->translate( 0,0,-600 );
  camLogo->setNearClipPlane( 10.0f );
  camLogo->setFarClipPlane( 1000.0f );

  lightLogo = new SpotLight( Vector3(0,0,-500), Vector3(0,0,1), 60, 0 );
  sceneLogo->addChild( lightLogo );

  //Setup Bub scene

  loadPackage( "bub.pak" );
  scene = new Scene;
  
  skinActor = new SkinMeshActor;
  skinActor->setMaterial( &mm );
  skinActor->setMesh( character );
  skinActor->translate( 60, 0, 0 );
  scene->addChild( skinActor );

  skinActor2 = new SkinMeshActor;
  skinActor2->setMaterial( &mm2 );
  skinActor2->setMesh( character );
  skinActor2->translate( -60, 0, 0 );
  scene->addChild( skinActor2 );

  TriMesh *cubeMesh = new CubeMesh;
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( &matWhite );
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

  cam3D.setCenter( center );
  cam3D.translate( 0,0,-400 );
  cam3D.orbitV( Util::DegToRad( 20 ), true );
  cam3D.orbitH( Util::DegToRad( 30 ), true );
  cam3D.setNearClipPlane( 10.0f );
  cam3D.setFarClipPlane( 1000.0f );

  light = new SpotLight( Vector3(-200,200,-200), Vector3(1,-1,1), 60, 0 );
  //light->setColor( Vector3( 1,.2,.2 ) );
  scene->addChild( light );

  Light* l = new SpotLight( Vector3(200,200,-200), Vector3(-1,-1,1), 60, 0 );
  //l->setColor( Vector3( .2,.2,1 ) );
  scene->addChild( l );

  //Start with Logo scene
  sceneRender = sceneLogo;
  lightRender = lightLogo;
  camRender = camLogo;

  //sceneRender = scene;
  //lightRender = light;
  //camRender = &cam3D;

  ///////////////////////////////////////
  //Test triangulation
  PolyMesh p;
  int numVerts = 6;
  ArrayList< PolyMesh::Vertex* > verts;
  for (int v=0; v<numVerts; ++v)
    verts.pushBack( p.addVertex() );
  p.addFace( verts.buffer(), numVerts );

  verts[0]->point.set( 0,0,0 );
  verts[1]->point.set( 1,0,0 );
  verts[2]->point.set( 1,1,0 );
  verts[3]->point.set( 2,1,0 );
  verts[4]->point.set( 2,2,0 );
  verts[5]->point.set( 0,2,0 );
  p.triangulate();

  PolyMeshActor *pa = new PolyMeshActor;
  pa->setMesh( &p );
  pa->scale( 100, 100, 100 );
  //scene->addChild( pa );

  /////////////////////////////////
  
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
