#include <geEngine.h>
#include <geGLHeaders.h>
#include <geClass.h>
using namespace GE;
using namespace OCC;

#include <cstdlib>
#include <cstdio>

void applyFK (int frame);

class SPolyMesh : public PolyMesh
{
  DECLARE_SUBCLASS (SPolyMesh, PolyMesh); DECLARE_END;

public:

  class Vertex; class HalfEdge; class Edge; class Face;

  class Vertex : public VertexBase <SPolyMesh,PolyMesh> {
    DECLARE_SUBCLASS (Vertex, PolyMesh::Vertex); DECLARE_END;
  public:
    Uint32 boneIndex [4];
    Float boneWeight [4];
  };
  
  class HalfEdge : public HalfEdgeBase <SPolyMesh,PolyMesh> {
    DECLARE_SUBCLASS (HalfEdge, PolyMesh::HalfEdge); DECLARE_END;
  };
  
  class Edge : public EdgeBase <SPolyMesh,PolyMesh> {
    DECLARE_SUBCLASS (Edge, PolyMesh::Edge); DECLARE_END;
  };
  
  class Face : public FaceBase <SPolyMesh,PolyMesh> {
    DECLARE_SUBCLASS (Face, PolyMesh::Face); DECLARE_END;
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

DEFINE_CLASS (SPolyMesh);
DEFINE_CLASS (SPolyMesh::Vertex);
DEFINE_CLASS (SPolyMesh::HalfEdge);
DEFINE_CLASS (SPolyMesh::Edge);
DEFINE_CLASS (SPolyMesh::Face);


class SPolyActor : public PolyMeshActor { public:
  virtual void renderMesh (MaterialId materialId);
};


enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

ByteString data;
MaxCharacter *character;

SPolyActor *actor;
FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Renderer renderer;
bool down3D;
Vector2 lastMouse3D;
int boneColorIndex = 0;
int frame = 0;

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
    //boneColorIndex++;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    if (frame < 61) applyFK (++frame);
    break;
  case '-':
    //if (boneColorIndex > 0)
    //  boneColorIndex--;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    if (frame > 0) applyFK (--frame);
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

  PolyMesh *mesh = actor->getMesh();
  for (PolyMesh::VertIter v(mesh); !v.end(); ++v) {
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

void SPolyActor::renderMesh (MaterialId matid)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  TexMesh::FaceIter uf(texMesh);
  for (SPolyMesh::FaceIter f(polyMesh); !f.end(); ++f, ++uf) {
    
    //Check if this face belongs to current material
    if (f->materialId() != matid && matid != GE_ANY_MATERIAL_ID)
      continue;
    
    glBegin(GL_POLYGON);
    
    //Use face normal for all vertices in flat mode
    if (polyMesh->getShadingModel() == SHADING_FLAT)
      glNormal3fv ((Float*)&f->normal);
    
    TexMesh::FaceVertIter uv(*uf);
    for(SPolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
      
      //Interpolate per-vertex normals in smooth mode
      if (polyMesh->getShadingModel() == SHADING_SMOOTH)
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

PolyMesh* loadPackage (String fileName)
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
  character = (MaxCharacter*) sm.deserialize ((void*)data.buffer());  
  SkinMesh *inMesh = character->mesh;
  
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
    
    SPolyMesh::Face *face = (SPolyMesh::Face*) polyMesh->addFace (corners, numCorners);
    if (face != NULL) face->smoothGroups = inMesh->faces->at(f).smoothGroups;

    delete[] corners;
  }

  polyMesh->updateNormals ();
  return polyMesh;
}

void applyFK (int frame)
{
  SkinPose *pose = character->pose;
  SkinAnim *anim = character->anim;
  ArrayList <Matrix4x4> fkMats;
  ArrayList <Matrix4x4> skinMats;
  int cindex = 1;

  int numTracks = anim->tracks->size();
  int numKeys = anim->tracks->first()->keys->size();
  
  //Root FK matrix = local matrix
  Matrix4x4 rootWorld;
  //rootWorld.fromQuaternion (skel->bones->first().localRot);
  rootWorld.fromQuaternion (anim->tracks->first()->keys->at(frame).value);
  rootWorld.setColumn (3, pose->bones->first().localTra);
  fkMats.pushBack (rootWorld);
  
  //Walk all the bones
  for (int b=0; b<pose->bones->size(); ++b)
  {
    //Final skin matrix = FK matrix * world matrix inverse
    SkinBone *parent = &pose->bones->at(b);
    skinMats.pushBack (fkMats[b] * parent->worldInv);
    
    //Walk the children
    for (Uint32 c=0; c<parent->numChildren; ++c)
    {
      //Child FK matrix = parent FK matrix * local matrix
      SkinBone *child = &pose->bones->at (cindex);
      SkinTrack *track = anim->tracks->at (cindex);
      cindex++;
      
      Matrix4x4 childLocal;
      //childLocal.fromQuaternion (child->localRot);
      childLocal.fromQuaternion (track->keys->at(frame).value);
      childLocal.setColumn (3, child->localTra);
      fkMats.pushBack (fkMats[b] * childLocal);
    }
  }
  
  SkinMesh *mesh = character->mesh;
  SPolyMesh *pmesh = (SPolyMesh*) actor->getMesh ();
  int vindex = 0;
  
  for (SPolyMesh::VertIter v(pmesh); !v.end(); ++v, ++vindex)
  {
    v->point.set (0,0,0);
    for (int i=0; i<4; ++i)
    {
      Vector3 &posePoint = mesh->verts->at(vindex).point;
      v->point += skinMats[v->boneIndex[i]] * posePoint * v->boneWeight[i];
    }
  }
}

class DD
{
  DECLARE_SERIAL_CLASS (DD);
  DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
  DECLARE_END;
public:
  int d;
  DD () {}
  DD (SM *sm) {}
  void serialize (void *sm)
  {
    int poke = 1;
  }
};

DEFINE_CLASS (DD);

class CC
{
  DECLARE_SERIAL_CLASS (CC);
  DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
  DECLARE_END;
public:
  ResPtrArrayList<DD*> *list;
  CC () {list = new ResPtrArrayList<DD*>;}
  CC (SM *sm) {}
  void serialize (void *sm)
  {
    ((SM*)sm)->resourcePtr (Class(GenArrayList), (void**)&list);
  }
};

DEFINE_CLASS (CC);


int main (int argc, char **argv)
{
  /*
  CC cc;
  for (int d=0; d<5; ++d) {
    cc.list->pushBack (new DD);
    cc.list->last()->d = d;
  }

  SM sm;
  void *data;
  UintP size;
  sm.serialize (&cc, &data, &size);

  CC *ccc = (CC*) sm.deserialize (data);
  */

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
  
  //VertColorMaterial mat;
  //StandardMaterial mat;
  PhongMaterial mat;
  mat.setSpecularity (0.5);
  
  //StandardMaterial mat;
  //mat.setCullBack (false);
  //mat.setUseLighting (false);
  
  actor = new SPolyActor;
  actor->setMaterial (&mat);
  actor->setMesh (loadPackage ("bub3.pak"));
  applyFK (1);
  
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
