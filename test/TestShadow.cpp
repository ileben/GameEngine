#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
using namespace GE;

#include <cstdlib>
#include <cstdio>

/*
==========================================================
State
==========================================================*/

enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

Actor *scene;
FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Renderer renderer;
bool down3D;
Vector2 lastMouse3D;
UintSize frame = 0;
UintSize numFrames = 0;
float curTime = 0.0f;
float maxTime = 0.0f;

int resY = 512;
int resX = 512;
Vector3 center(0, 0, 0);
CameraMode cameraMode;

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
  
  Float angleH = -diff.x * (2*PI) / 400;
  Float angleV = -diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom = -diff.y * ( eyeDist * 0.01f );
  lastMouse3D.set( (Float)x, (Float)y );
  
  switch (cameraMode)
  {  
  case CAMERA_MODE_ZOOM:

    cam3D.zoom( zoom );
    break;
    
  case CAMERA_MODE_ORBIT:
    
    cam3D.setCenter( center );
    cam3D.orbitH( angleH, true );
    cam3D.orbitV( angleV, true );
    break;

  case CAMERA_MODE_PAN:

    cam3D.panH( panH );
    cam3D.panV( panV );
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
    cameraMode = CAMERA_MODE_ORBIT;
  }
  else if ((mods & GLUT_ACTIVE_CTRL) ||
           button == GLUT_RIGHT_BUTTON)
  {
    cameraMode = CAMERA_MODE_ZOOM;
  }
  else
  {
    cameraMode = CAMERA_MODE_PAN;
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
  switch (key)
  {
  case 27:
    //Quit on escape
    exit(0);
  }
}

void renderAxes ()
{
  StandardMaterial mat;
  mat.setUseLighting( false );
  mat.begin();

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glScalef( 100, 100, 100 );

  glPopMatrix();
  
  mat.end();
}

class PointMeshActor : public TriMeshActor
{ public:
  void renderMesh( MaterialID materialID )
  {
    ArrayList< TriMeshVertex > *verts =
      (ArrayList< TriMeshVertex >*) &mesh->data;

    glBegin( GL_POINTS );
    
    for (UintSize v=0; v<verts->size(); ++v)
    {
      Vector3 p = verts->at(v).point;
      glVertex3fv( (GLfloat*) &verts->at(v).point );
    }
    
    glEnd();
  }
};

void display ()
{
  renderer.begin();
  
  //Switch 3D camera
  renderer.setViewport( 0,0,resX, resY );
  renderer.setCamera( &cam3D );
  
  //Render 3D stuff
  renderer.renderActor( scene );
  
  //Render 2D frames per second
  renderer.setViewport( 0,0,resX, resY );
  renderer.setCamera( &cam2D );
  renderer.renderWidget( &lblFps );
  
  renderer.end();
}

void reshape (int w, int h)
{
  resX = w;
  resY = h;
}

void findCenter ()
{
  int count = 0;
  center.set (0,0,0);
  /*
  PolyMesh *mesh = polyActor->getMesh();
  for (PolyMesh::VertIter v(mesh); !v.end(); ++v) {
    center += v->point;
    count++;
  }
  
  center /= (Float)count; */
}

void cleanup()
{
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Test Shadow" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutMouseFunc( click );
  glutMotionFunc( drag );
  glutIdleFunc( display );
  idleDraw = true;
  
  Float L = 0.9f;
  GLfloat ambient[4] = {0.2f,0.2f,0.2f, 1.0f};
  GLfloat diffuse[4] = {L, L, L, 1.0f};
  GLfloat position[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
  glLightfv( GL_LIGHT0, GL_POSITION, position );
  glEnable(  GL_LIGHT0 );
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  printf( "Kernel loaded\n" );
  
  //Setup camera
  cam3D.setCenter( center );
  cam3D.translate( 0,0,200 );
  cam3D.orbitV( Util::DegToRad( -20 ), true );
  cam3D.orbitH( Util::DegToRad( +45 ), true );
  cam3D.setNearClipPlane( 10.0f );
  cam3D.setFarClipPlane( 1000.0f );
  
  //VertColorMaterial mat;
  StandardMaterial mat;
  //PhongMaterial mat;
  //mat.setUseLighting( false );
  mat.setSpecularity( 0.5 );
  
  scene = new Actor;

  TriMesh *sphereMesh = new SphereMesh( 40 );
  TriMeshActor *sphere = new TriMeshActor;
  sphere->setMaterial( &mat );
  sphere->setMesh( sphereMesh );
  sphere->scale( 50 );
  scene->addChild( sphere );
  
  TriMesh *cubeMesh = new CubeMesh();
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( &mat );
  cube->setMesh( cubeMesh );
  cube->scale( 40 );
  scene->addChild( cube );

  StandardMaterial axesMat;
  axesMat.setUseLighting( false );
  AxisActor *axes = new AxisActor;
  axes->scale( 100 );
  axes->setMaterial( &axesMat );
  scene->addChild( axes );

  
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
