#include <geEngine.h>
#include <geGLHeaders.h>
#include <geClass.h>
using namespace GE;
using namespace OCC;

#include <cstdlib>
#include <cstdio>


class SPolyMesh : public DMesh
{
  DECLARE_SUBCLASS (SPolyMesh, DMesh); DECLARE_END;

public:

  class Vertex; class HalfEdge; class Edge; class Face;

  class Vertex : public VertexBase <SPolyMesh,DMesh> {
    DECLARE_SUBCLASS (Vertex, DMesh::Vertex); DECLARE_END;
  public:
    Uint32 boneIndex [4];
    Float boneWeight [4];
  };
  
  class HalfEdge : public HalfEdgeBase <SPolyMesh,DMesh> {
    DECLARE_SUBCLASS (HalfEdge, DMesh::HalfEdge); DECLARE_END;
  };
  
  class Edge : public EdgeBase <SPolyMesh,DMesh> {
    DECLARE_SUBCLASS (Edge, DMesh::Edge); DECLARE_END;
  };
  
  class Face : public FaceBase <SPolyMesh,DMesh> {
    DECLARE_SUBCLASS (Face, DMesh::Face); DECLARE_END;
  };
  
  #include "geHmeshDataiter.h"
  #include "geHmeshAdjiter.h"
  
  SPolyMesh() {
    setClasses(
      Class(Vertex),
      Class(HalfEdge),
      Class(Edge),
      Class(Face));
  }
};

class SPolyActor : public Shape
{
public:
  virtual void renderDynamic (MaterialId materialId);
};

DEFINE_CLASS (SPolyMesh);
DEFINE_CLASS (SPolyMesh::Vertex);
DEFINE_CLASS (SPolyMesh::HalfEdge);
DEFINE_CLASS (SPolyMesh::Edge);
DEFINE_CLASS (SPolyMesh::Face);


enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

ByteString data;
MaxCharacter_Res *character;

SPolyActor *actor;
FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Renderer renderer;
bool down3D;
Vector2 lastMouse3D;
int boneColorIndex = 0;

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
  
  Vector2 diff = Vector2((Float)x,(Float)y) - lastMouse3D;
  float eyeDist = (cam3D.getEye() - center).norm();
  
  Float angleH = -diff.x * (2*PI) / 400;
  Float angleV = -diff.y * (2*PI) / 400;
  Float panH = -diff.x * (eyeDist * 0.002f);
  Float panV = diff.y * (eyeDist * 0.002f);
  Float zoom = -diff.y * (eyeDist * 0.01f);
  lastMouse3D.set ((Float)x, (Float)y);
  
  switch (cameraMode)
  {  
  case CAMERA_MODE_ZOOM:

    cam3D.zoom (zoom);
    break;
    
  case CAMERA_MODE_ORBIT:
    
    cam3D.setCenter (center);
    cam3D.orbitH (angleH, true);
    cam3D.orbitV (angleV, true);
    break;

  case CAMERA_MODE_PAN:

    cam3D.panH (panH);
    cam3D.panV (panV);
    break;
  }
  
  postRedisplay ();
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
  
  lastMouse3D.set((Float)x, (Float)y);
  down3D = true;
}

void click (int button, int state, int x, int y)
{
  click3D (button, state, x, y);
}

void drag (int x, int y)
{
  if (down3D)
    drag3D (x, y);
}

void keyboard (unsigned char key, int x, int y)
{
  switch (key)
  {
  case '+':
    boneColorIndex++;
    break;
  case '-':
    if (boneColorIndex > 0)
      boneColorIndex--;
    break;
  case 27:
    //Quit on escape
    exit(0);
  }
}

void renderAxes ()
{
  StandardMaterial mat;
  mat.setUseLighting (false);
  mat.begin ();

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glScalef (100, 100, 100);
  
  glBegin (GL_LINES);
  glColor3f  (1, 0, 0);
  glVertex3f (0, 0, 0);
  glVertex3f (1, 0, 0);

  glColor3f  (0, 1, 0);
  glVertex3f (0, 0, 0);
  glVertex3f (0, 1, 0);

  glColor3f  (0, 0, 1);
  glVertex3f (0, 0, 0);
  glVertex3f (0, 0, 1);
  glEnd ();

  glPopMatrix ();
  
  mat.end ();
}

void display ()
{
  renderer.begin();
  
  //switch camera
  renderer.setViewport (0,0,resX, resY);
  renderer.setCamera (&cam3D);
  
  //draw model
  renderer.drawActor (actor);
  renderAxes ();
  
  //Frames per second
  renderer.setViewport (0,0,resX, resY);
  renderer.setCamera (&cam2D);
  renderer.drawWidget (&lblFps);
  
  renderer.end();
}

void reshape (int w, int h)
{
}

void findCenter ()
{
  int count = 0;
  center.set (0,0,0);

  DMesh *mesh = actor->getDynamic();
  for (DMesh::VertIter v(mesh); !v.end(); ++v) {
    center += v->point;
    count++;
  }
  
  center /= (Float)count;
}

void cleanup()
{
}

void initGlut (int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

  glutInitWindowPosition(100,100);
  glutInitWindowSize(resX,resY);
  glutCreateWindow("Test Skin");
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(click);
  glutMotionFunc(drag);
  glutIdleFunc(display);
  idleDraw = true;
  
  Float L = 0.9f;
  GLfloat ambient[4] = {0.2f,0.2f,0.2f, 1.0f};
  GLfloat diffuse[4] = {L, L, L, 1.0f};
  GLfloat position[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glEnable(GL_LIGHT0);
}

class VertColorMaterial : public StandardMaterial { public:
  virtual void begin ()  {
    StandardMaterial::begin ();
    glEnable (GL_COLOR_MATERIAL);
  }
};

void SPolyActor::renderDynamic (MaterialId matid)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  UMesh::FaceIter uf(uvMesh);
  for (SPolyMesh::FaceIter f(dynMesh); !f.end(); ++f, ++uf) {
    
    //Check if this face belongs to current material
    if (f->materialId() != matid && matid != GE_ANY_MATERIAL_ID)
      continue;
    
    glBegin(GL_POLYGON);
    
    //Use face normal for all vertices in flat mode
    if (dynMesh->getShadingModel() == SHADING_FLAT)
      glNormal3fv ((Float*)&f->normal);
    
    /* //TODO: Remove when sure that no mesh file can break HMesh
    glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
    for(DMesh::FaceHedgeIter fh(*f); !fh.end(); ++fh)
      if (fh->tag.id == 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f);*/
    
    UMesh::FaceVertIter uv(*uf);
    for(SPolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
      
      //Interpolate per-vertex normals in smooth mode
      if (dynMesh->getShadingModel() == SHADING_SMOOTH)
        glNormal3fv ((Float*)&h->smoothNormal()->coord);

      SPolyMesh::Vertex *vert = h->dstVertex();
      glColor3f (1,1,1);
      for (int b=0; b<4; ++b) {
        if (vert->boneIndex [b] == boneColorIndex &&
            vert->boneWeight [b] > 0.0f)
            glColor3f (1, 0, 0); }

      
      //UV coordinates
      if (!uv.end()) {
        glTexCoord2f (uv->point.x, uv->point.y); }
      
      //Vertex coordinate
      glVertex3fv ((Float*)&h->dstVertex()->point);
    }
    
    glEnd();
  }
}

DMesh* loadPackage (String fileName)
{
  
  FileRef file = new File (fileName);
  if (!file->open ("rb"))
  {
    printf ("Failed opening file!\n");
    getchar ();
    return 0;
  }
  
  data = file->read (file->getSize());
  file->close();
  
  SerializeManager sm;
  character = (MaxCharacter_Res*) sm.deserialize ((void*)data.buffer());  
  SkinPolyMesh_Res *inMesh = character->mesh;
  
  /*
  void *data; UintP size;
  SkinPolyMesh_Res outMesh;

  SerializeManager sm;
  sm.serialize (&outMesh, &data, &size);
  
  SkinPolyMesh_Res *inMesh =
    (SkinPolyMesh_Res*) sm.deserialize (data);
  */
  printf ("Imported %d verts, %d faces, %d indices\n",
          inMesh->verts->size(),
          inMesh->faces->size(),
          inMesh->indices->size());
  
  SPolyMesh *polyMesh = new SPolyMesh;
  ArrayList <SPolyMesh::Vertex*> verts (inMesh->verts->size());
  for (int v=0; v<inMesh->verts->size(); ++v)
  {
    SPolyMesh::Vertex *vert = (SPolyMesh::Vertex*) polyMesh->addVertex ();
    vert->point = inMesh->verts->at(v).point;
    for (int b=0; b<4; ++b) {
      vert->boneIndex[b] = inMesh->verts->at(v).boneIndex[b];
      vert->boneWeight[b] = inMesh->verts->at(v).boneWeight[b]; }
    verts.pushBack (vert);
  }
  
  int nextIndex = 0;
  for (int f=0; f<inMesh->faces->size(); ++f)
  {
    int numCorners = inMesh->faces->at(f).numCorners;
    HMesh::Vertex **corners = new HMesh::Vertex* [numCorners];
    
    for (int c=0; c<numCorners; ++c) {
      int vertIndex = inMesh->indices->at (nextIndex++);
      if (vertIndex > inMesh->verts->size()) {
        printf ("Invalid vertex: %d\n", vertIndex);
        vertIndex = 0; }
      corners [c] = verts [vertIndex];
    }
    
    DMesh::Face *face = (DMesh::Face*) polyMesh->addFace (corners, numCorners);
    if (face != NULL) face->smoothGroups = inMesh->faces->at(f).smoothGroups;

    delete[] corners;
  }

  polyMesh->updateNormals ();
  return polyMesh;
}

void applyFK ()
{
  Skeleton_Res *skel = character->skeleton;
  int cindex = 1;

  ArrayList <Matrix4x4> worldMats;
  
  for (int b=0; b<skel->bones->size(); ++b)
  {
    SkeletonBone *parent = &skel->bones->at(b);
    for (Uint32 c=0; c<parent->numChildren; ++c)
    {
      SkeletonBone *child = &skel->bones->at (cindex++);
      child->poseRot = parent->poseRot * child->poseRot;
    }
    
    Matrix4x4 worldMat;
    worldMat.fromQuaternion (parent->poseRot);
    worldMats.pushBack (worldMat);
  }

  SPolyMesh *mesh = (SPolyMesh*) actor->getDynamic ();
  for (SPolyMesh::VertIter v(mesh); !v.end(); ++v)
  {
    Vector3 result (0,0,0);
    
    for (int i=0; i<4; ++i)
    {
      Uint32 boneIndex = v->boneIndex[i];
      SkeletonBone *bone = &skel->bones->at (boneIndex);
      Matrix4x4 &worldMat = worldMats [boneIndex];
      Vector3 fraction = worldMat * bone->poseInv * v->point;
      result += fraction * v->boneWeight[i];
    }

    v->point = result;
  }
}

int main (int argc, char **argv)
{
  //Initialize GLUT
  initGlut(argc, argv);
  
  Kernel kernel;
  kernel.enableVerticalSync(false);
  printf("Kernel loaded\n");  
  
  //Setup camera
  cam3D.setCenter(center);
  cam3D.translate(0,0,200);
  cam3D.orbitV (Util::DegToRad (-20), true);
  cam3D.orbitH (Util::DegToRad (-30), true);
  cam3D.setNearClipPlane(10.0f);
  cam3D.setFarClipPlane(1000.0f);
  
  VertColorMaterial mat;
  //StandardMaterial mat;
  //PhongMaterial mat;
  //mat.setSpecularity (0.5);
  
  //StandardMaterial mat;
  //mat.setCullBack (false);
  //mat.setUseLighting (false);
  
  actor = new SPolyActor;
  actor->setMaterial (&mat);
  actor->setDynamic (loadPackage ("bub.pak"));
  //applyFK ();
  
  lblFps.setLocation (Vector2 (0.0f,(Float)resY));
  lblFps.setColor (Vector3 (1.0f,1.0f,1.0f));
  
  //Find model center
  findCenter();
  cam3D.setCenter (center);

  //Run application
  atexit(cleanup);
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
