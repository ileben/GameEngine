#include <geEngine.h>
#include <geGLHeaders.h>
using namespace GE;
using namespace OCC;

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <stdio.h>

enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

Renderer renderer;
Camera3D cam3D;
Shape *shape;

int resY = 512;
int resX = 512;
Vector3 center(0, 30, 0);
CameraMode cameraMode;

Vector2 lastMouse3D;
bool down3D = false;

void drag3D(int x, int y)
{
  if (down3D) {

    Vector2 diff = Vector2(x,y) - lastMouse3D;
    Float angleH = -diff.x * (2*PI) / 400;
    Float angleV = -diff.y * (2*PI) / 400;
    Float panH = -diff.x / 5;
    Float panV = diff.y / 5;
    Float zoom = -diff.y;
    lastMouse3D.set(x, y);

    switch(cameraMode) {
    case CAMERA_MODE_ZOOM:

      cam3D.zoom(zoom);
      break;

    case CAMERA_MODE_ORBIT:
      
      cam3D.setCenter(center);
      cam3D.orbitH(angleH, true);
      cam3D.orbitV(angleV, true);
      break;

    case CAMERA_MODE_PAN:

      cam3D.panH(panH);
      cam3D.panV(panV);
      break;
    }

    glutPostRedisplay();
  }
}

void click3D(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {

    int mods = glutGetModifiers();

    if (mods & GLUT_ACTIVE_SHIFT)
      cameraMode = CAMERA_MODE_ORBIT;
    else if (mods & GLUT_ACTIVE_CTRL)
      cameraMode = CAMERA_MODE_ZOOM;
    else
      cameraMode = CAMERA_MODE_PAN;

    lastMouse3D.set(x, y);
    down3D = true;

  }else{
    down3D = false;
  }
}

void click(int button, int state, int x, int y)
{
  click3D(button, state, x, y);
}

void drag(int x, int y)
{
  if (down3D)
    drag3D(x, y);
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 27:
    //Quit on escape
    exit(0);
  }
}


void reshape(int w, int h)
{
  resX = w; resY = h;
  renderer.setViewport(0,0,w,h);
  renderer.setCamera(&cam3D);
}

void display()
{
  renderer.begin();
  renderer.drawShape(shape);
  renderer.end();
}

void findCenter()
{
  int count = 0;
  center.set(0,0,0);
  DynamicMesh *mesh = shape->getDynamic();
  for (DynamicMesh::VertIter v(mesh); !v.end(); ++v) {
    center += v->point;
    count++;
  }

  center /= count;
}

void initGlut(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);

  glutInitWindowPosition(100,100);
  glutInitWindowSize(resX,resY);
  glutCreateWindow("Game Engine");

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(click);
  glutMotionFunc(drag);

  Float L = 0.9;
  GLfloat ambient[4] = {0.5, 0.5, 0.5, 1};
  GLfloat diffuse[4] = {L, L, L, 1};
  GLfloat position[4] = {0, 0, 0, 1};
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glEnable(GL_LIGHT0);
}

int main(int argc, char **argv)
{
  //Initialize GLUT
  initGlut(argc, argv);
  Kernel kernel;

  //Setup camera
  cam3D.setEye(Vector3(0,20,100));
  cam3D.setCenter(center);
  renderer.setBackColor(Vector3(0.5,0.5,0.5));

  //Load shape model
  LoaderObj ldr;
  FileRef f = new File("smooth.obj");

  int start = Time::GetTicks();
  ldr.loadFile(f->getPathName());
  int interval = Time::GetTicks() - start;
  printf("interval: %d msec\n", interval);

  shape = (Shape*)ldr.getFirstObject(OBJECT_SHAPE);
  if (shape == NULL) return EXIT_FAILURE;

  //Find model center
  findCenter();

  //Create material using texture
  Material mat;
  mat.setDiffuseColor(Vector3(1,0,0));
  //mat.setSpecularity(1.0);
  //mat.setGlossiness(0.2);
  shape->setMaterial(&mat);

  //Run application
  glutMainLoop();

  return EXIT_SUCCESS;
}
