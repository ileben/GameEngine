#include <geEngine.h>
#include <geGLHeaders.h>
#include <geClass.h>
using namespace GE;
using namespace OCC;

#include <cstdlib>
#include <cstdio>


class Testy : public Object
{
public:
  DECLARE_SUBCLASS (Testy, Object);
  DECLARE_PROPERTY (int, a);
  DECLARE_PROPERTY (int, b);
  DECLARE_CALLBACK (CLSEVT_CREATE, createMe);
  DECLARE_END;
  
public:
  
  int a;
  int b;

  void createMe (void *param)
  {
    printf ("Creating Testy!\n");
  }
};

DEFINE_CLASS (Testy);


class SubTesty : public Testy
{
  DECLARE_SUBCLASS (SubTesty, Testy);
  DECLARE_PROPERTY (int, c);
  DECLARE_END;
  
public:
  
  int c;
  
  /*
  PROPBEGIN (Testy);
  PROPERTY  (c);
  PROPEND;
  
  DEFAULT (int, c, 55);
  DEFAULT (int, b, 440); */
};

DEFINE_CLASS (SubTesty);


class ETexMesh : public TexMesh
{
  DECLARE_SUBCLASS (ETexMesh, TexMesh);
  DECLARE_END;
  
public:

  class Vertex; class HalfEdge; class Edge; class Face;

  class Vertex : public VertexBase <ETexMesh,TexMesh> {
    DECLARE_SUBCLASS (Vertex, TexMesh::Vertex); DECLARE_END;
  public:
    bool selected;
    Vertex() : selected(false) {};
  };

  class HalfEdge : public HalfEdgeBase <ETexMesh,TexMesh> {
    DECLARE_SUBCLASS (HalfEdge, TexMesh::HalfEdge); DECLARE_END;
  };

  class Edge : public EdgeBase <ETexMesh,TexMesh> {
    DECLARE_SUBCLASS (Edge, TexMesh::Edge); DECLARE_END;
  };

  class Face : public FaceBase <ETexMesh,TexMesh> {
    DECLARE_SUBCLASS (Face, TexMesh::Face); DECLARE_END;
  };

  ETexMesh() {
    setClasses(
      Class(Vertex),
      Class(HalfEdge),
      Class(Edge),
      Class(Face));
  }

  #include "geHmeshDataiter.h"
  #include "geHmeshAdjiter.h"
};

DEFINE_CLASS (ETexMesh);
DEFINE_CLASS (ETexMesh::Vertex);
DEFINE_CLASS (ETexMesh::HalfEdge);
DEFINE_CLASS (ETexMesh::Edge);
DEFINE_CLASS (ETexMesh::Face);


enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

Image imgDiff;
Texture *texDiff = NULL;
Texture *texSpec = NULL;
PolyMeshActor *zekko = NULL;
ETexMesh *uvmesh = NULL;
TriMesh smesh;
FpsLabel lblFps;

Camera3D cam3D;
Camera2D cam2D;
Renderer renderer;

int resX3D = 512;
int resX2D = 512;
int resY = 512;
int resX = resX3D + resX2D;
Vector3 center(0, 30, 0);
CameraMode cameraMode;

int paintSize = 4;
int eraseSize = 20;
const char *paintTitle = "Texture Painter (paint)";
const char *eraseTitle = "Texture Painter (erase)";
int *brushSize = &paintSize;
bool paint = true;

LinkedList<ETexMesh::Vertex*> selection;
bool edit = false;

Vector2 mouse2D;
Vector2 lastMouse2D;
Vector2 lastMouse3D;
bool moved2D = false;
bool down3D = false;
bool down2D = false;


//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}


void saveImage()
{
  //Colors for background and lines
  Color white(1,1,1);
  Color black(0,0,0);
  
  //Create new image for UV mesh
  Image imgUV;
  imgUV.create(imgDiff.getWidth(), imgDiff.getHeight(), COLOR_FORMAT_RGB, white);

  //Draw UV mesh
  for (ETexMesh::EdgeIter e(uvmesh); !e.end(); ++e) {
    Vector2 uv1 = e->vertex1()->point;
    Vector2 uv2 = e->vertex2()->point;
    uv1.x *= imgDiff.getWidth(); uv1.y *= imgDiff.getHeight();
    uv2.x *= imgDiff.getWidth(); uv2.y *= imgDiff.getHeight();
    imgUV.drawLine(uv1.x, uv1.y, uv2.x, uv2.y, black);
  }

  //Save UV and texture image
  EncoderParamsJPEG params;
  params.quality = 100;

  if (imgUV.writeFile("mesh.jpg", &params, "JPEG") != IMAGE_NO_ERROR)
    printf("Failed saving mesh image file!\n");
  else printf("Mesh image saved successfully.\n");

  if (imgDiff.writeFile("texture.jpg", &params, "JPEG") != IMAGE_NO_ERROR)
    printf("Failed saving texture image file!\n");
  else printf("Texture image saved successfully.\n");

  //Save mesh
  SaverObj sav;
  sav.addObject(zekko);
  FileRef f = new File("output.obj");

  if (sav.saveFile(f->getPathName()) == false)
    printf("Failed saving UV mesh!\n");
  else printf("UV mesh saved successfully.\n");
}

void drawPixel(int x, int y)
{
  Color src;
  float W = (float)(*brushSize)/2 - 0.5f;
  int S = (*brushSize)/2+1;
  Vector2 c(x+0.5f, y+0.5f);

  if (paint) src.set(0.1f, 0.1f, 0.1f);
  else       src.set(1.0f, 1.0f, 1.0f);

  for (int X=x-S; X<=x+S; ++X) {
    for (int Y=y-S; Y<=y+S; ++Y) {
      
      Vector2 v(X+0.5f, Y+0.5f);
      Vector2 d(v.x - c.x, v.y - c.y);
      float dist = d.norm();
      float A = 1.0f;
      if (dist > W) A = 1.0f - (dist-W);
      if (A <= 0.0f) continue;

      Color out, dst = imgDiff.getPixel(X, Y);
      out.r = (A * src.r + (1.0f - A) * dst.r);
      out.g = (A * src.g + (1.0f - A) * dst.g);
      out.b = (A * src.b + (1.0f - A) * dst.b);
      out.a = 1.0f;
      imgDiff.setPixel(X, Y, out);
    }}

  texDiff->updateRegion(0,0, &imgDiff);
}

void selectVertex(ETexMesh::Vertex *v)
{
  v->selected = true;
  selection.pushBack(v);
}

void clearSelection()
{
  for (LinkedList<ETexMesh::Vertex*>::Iterator s=selection.begin();
    s!=selection.end(); ++s) (*s)->selected = false;
  selection.clear();
}

ETexMesh::Vertex* pickVertex(int x, int y)
{
  for (ETexMesh::VertIter v(uvmesh); !v.end(); ++v) {
    int vx = (int)(v->point.x * resX2D);
    int vy = (int)(v->point.y * resY);
    if (::abs(vx - x) <= 5 && ::abs(vy - y) <= 5)
    {return *v;}
  }

  return NULL;
}

void moveVertex(int x, int y)
{
  Vector2 diff((float)(x - lastMouse2D.x)/resX2D,
                (float)(y - lastMouse2D.y)/resY);

  for (LinkedList<ETexMesh::Vertex*>::Iterator s=selection.begin();
    s!=selection.end(); ++s) {

    (*s)->point.x += diff.x;
    (*s)->point.y += diff.y;
  }
}

void grabVertexUp(int x, int y)
{
  Uint32 mods = glutGetModifiers();
  ETexMesh::Vertex *v = pickVertex(x,y);

  //Just skip if no vertex hit
  if (v == NULL) return;

  //Is shift key not pressed?
  if ((mods & GLUT_ACTIVE_SHIFT) == 0) {

    //Switch to single vertex
    clearSelection();
    selectVertex(v);
  }
}

void grabVertexDown(int x, int y)
{
  int mods = glutGetModifiers();
  ETexMesh::Vertex *v = pickVertex(x,y);
  
  //Clear selection if clicked out
  if (v == NULL) {
    clearSelection();
    return;
  }

  //Is shift key pressed?
  if (mods & GLUT_ACTIVE_SHIFT) {

    //Toggle vertex selection
    if (!v->selected) {
      v->selected = true;
      selection.pushBack(v);
    }else{
      v->selected = false;
      selection.remove(v);
    }

  }else{

    //Switch to single vertex if unselected
    if (!v->selected) {
      clearSelection();
      selectVertex(v); }
  }
}

void drag2D(int x, int y)
{
  if (down2D) {

    if (edit)
      moveVertex(x, y);
    else
      drawPixel(x, y);

    moved2D = true;
    lastMouse2D.set((Float)x, (Float)y);
  }

  mouse2D.set((Float)x, (Float)y);
  postRedisplay();
}

void click2D(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN && button == GLUT_LEFT) {

    down2D = true;
    moved2D = false;

    if (edit)
      grabVertexDown(x, y);
    else
      drawPixel(x, y);

    lastMouse2D.set((Float)x, (Float)y);
    postRedisplay();

  }else if (state == GLUT_UP) {

    if (edit && !moved2D)
      grabVertexUp(x,y);

    down2D = false;
    down3D = false;
  }
}

void move2D(int x, int y)
{
  mouse2D.set((Float)x, (Float)y);
  postRedisplay();
}

/*
void printMatrix(const Matrix4x4 &m)
{
  for (int r=0; r<4; ++r) {
    for (int c=0; c<4; ++c) {
      if (m.m[c][r] >= 0.0f) printf(" ");
      printf("%.4f ", m.m[r][c]); }
    printf("\n");
  }
}
*/

void printMatrix (const Matrix4x4 &m)
{
  for (int y=0; y<4; ++y) {
    for (int x=0; x<4; ++x)
      printf ("%.2f ", m.m[x][y]);
    printf ("\n");
  }
}

void printVector(const Vector3 &v)
{
  printf("(%f,%f,%f)\n", v.x, v.y, v.z);
}

void printQuat(const Quat &q)
{
  printf("(%f,%f,%f,%f)\n", q.x, q.y, q.z, q.w);
}

void drag3D(int x, int y)
{
  if (down3D) {

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
}

void click3D(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {

    int mods = glutGetModifiers();
    printf("mods: %d\n", mods);
    printf("button: %d\n", button);

    if (mods & GLUT_ACTIVE_SHIFT)
      cameraMode = CAMERA_MODE_ORBIT;
    else if (mods & GLUT_ACTIVE_CTRL ||
             button == GLUT_RIGHT_BUTTON)
      cameraMode = CAMERA_MODE_ZOOM;
    else
      cameraMode = CAMERA_MODE_PAN;

    lastMouse3D.set((Float)x, (Float)y);
    down3D = true;

  }else{

    down2D = false;
    down3D = false;
  }
}

void click(int button, int state, int x, int y)
{
  if (x < resX3D) {
    click3D(button, state, x, y);
  }else{
    x -= resX3D;
    click2D(button, state, x, y);
  }
}

void drag(int x, int y)
{
  if (down3D)
    drag3D(x, y);
  if (down2D)
    drag2D(x-resX3D, y);
}

void move(int x, int y)
{
  if (x < resX3D) {
  }else{
    x -= resX3D;
    move2D(x, y);
  }
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 9:
    //Tab toggles paint/erase
    paint = !paint;
    brushSize = (paint ? &paintSize : &eraseSize);
    glutSetWindowTitle( paint ? paintTitle : eraseTitle);
    postRedisplay();
    break;

  case 'e':
  case 'E':
    //Toggle drawing / uv editing
    edit = !edit;
    postRedisplay();
    break;

  case '+':
    //Increase brush size
    *brushSize += 1;
    postRedisplay();
    break;

  case '-':
    //Decrease brush size
    if (*brushSize > 1)
      *brushSize -= 1;
    postRedisplay();
    break;

  case 's':
  case 'S':
    //Save texture and mesh images
    saveImage();
    break;

  case 'r':
  case 'R':
    //Reload texture
    imgDiff.readFile("texture.jpg", "");
    texDiff->fromImage(&imgDiff);
    postRedisplay();
    break;

  case 27:
    //Quit on escape
    exit(0);
  }
}

void display()
{
  renderer.begin();
  
  //3D view (left)
  ///////////////////////////////////

  //TODO: remove following line - just testing purpose
  //zekko->getDynamic()->updateNormals();

  //switch camera
  renderer.setViewport(0,0,resX3D, resY);
  renderer.setCamera(&cam3D);

  //draw model
  renderer.drawActor(zekko);
  
  
  //2D view (right)
  ///////////////////////////////////
  
  Material::BeginDefault ();
  
  //switch camera
  renderer.setViewport(resX3D,0,resX2D,resY);
  renderer.setCamera(&cam2D);
  
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  
  //draw texture
  ///////////////////////////
  glColor3f(1,1,1);
  //glActiveTexture (GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texDiff->getHandle());
  
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2i(0, 0);
  glTexCoord2f(1, 0);
  glVertex2i(resX2D, 0);
  glTexCoord2f(1, 1);
  glVertex2i(resX2D, resY);
  glTexCoord2f(0, 1);
  glVertex2i(0, resY);
  glEnd();
  
  //overlay UV mesh
  /////////////////////////////
  
  //edges
  glColor3f(0,1,0);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINES);
  for (ETexMesh::EdgeIter e(uvmesh); !e.end(); ++e) {
  Vector2 uv1 = e->vertex1()->point;
  Vector2 uv2 = e->vertex2()->point;
  uv1.x *= resX2D; uv1.y *= resY;
  uv2.x *= resX2D; uv2.y *= resY;
  glVertex2fv((Float*)&uv1);
  glVertex2fv((Float*)&uv2); }
  glEnd();
  
  //vertices
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBegin(GL_QUADS);
  float R = 1.0f;
  for (ETexMesh::VertIter v(uvmesh); !v.end(); ++v) {
    
    Vector2 uv = (*v)->point;
    uv.x *= resX2D; uv.y *= resY;
    
    if (v->selected)
      {glColor3f(1,0,0); R=2.0f;}
    else {glColor3f(0,0,0); R=1.0f;}
    
    glVertex2f(uv.x-R, uv.y-R);
    glVertex2f(uv.x+R, uv.y-R);
    glVertex2f(uv.x+R, uv.y+R);
    glVertex2f(uv.x-R, uv.y+R);
  }
  glEnd();
  
  //draw brush
  if (!edit) {
    int S = (*brushSize)/2;
    int E = *brushSize - S;
    if (paint) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Vector2 M(mouse2D.x, mouse2D.y);
    
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
    glVertex2f(M.x-S, M.y-S);
    glVertex2f(M.x+E, M.y-S);
    glVertex2f(M.x+E, M.y+E);
    glVertex2f(M.x-S, M.y+E);
    glEnd();
  }

  //Frames per second
  renderer.setViewport(0,0,resX2D, resY);
  renderer.setCamera(&cam2D);
  renderer.drawWidget(&lblFps);

  renderer.end();
}

void reshape(int w, int h)
{
}


void findCenter()
{
  int count = 0;
  center.set(0,0,0);
  PolyMesh *mesh = zekko->getMesh();
  for (PolyMesh::VertIter v(mesh); !v.end(); ++v) {
    center += v->point;
    count++;
  }

  center /= (Float)count;
}

void cleanup()
{
  delete texSpec;
  delete texDiff;
}

void initGlut(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

  glutInitWindowPosition(100,100);
  glutInitWindowSize(resX,resY);
  glutCreateWindow("Texture Painter (paint)");

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(click);
  glutMotionFunc(drag);
  glutIdleFunc(display);
  glutPassiveMotionFunc(move);
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

int main (int argc, char **argv)
{  
  //Initialize GLUT
  initGlut(argc, argv);
  
  Kernel kernel;
  kernel.enableVerticalSync(false);
  printf("Kernel loaded\n");
  /*
  printf("Testy::Default a: %d\n", GetClassDefault (Testy, a));
  printf("Testy::Default b: %d\n", GetClassDefault (Testy, b));
  printf("SubTesty::Default b: %d\n", GetClassDefault (SubTesty, b));
  */
  //Testy* testy = (Testy*) kernel.spawn (Class(Testy));
  Testy* testy = (Testy*) kernel.spawn ("Testy");
  //printf("testy->default a: %d\n", GetDefault (testy, a));
  //printf("testy->default b: %d\n", GetDefault (testy, b));
  
  Testy *subtesty = (SubTesty*) kernel.spawn ("SubTesty");
  //printf("subtesty->default b: %d\n", GetDefault (subtesty, b));
  
  ByteString btesty;
  testy->a = 33;
  testy->b = 44;
  
  IClass::SaveText (testy, btesty);
  printf ("Testy:\n%s", btesty.buffer());
  
  ByteString ttesty =
    "a = 100; b = 200\n";
  
  IClass::LoadText (testy, ttesty, 0);
  IClass::Create (testy, NULL, 0);
  printf ("testy->a: %d\n", testy->a);
  printf ("testy->b: %d\n", testy->b);
  
  //Load shape model
  LoaderObj ldr;
  FileRef f = new File("zekko.obj");
  ldr.setUVMeshClass(Class(ETexMesh));
  
  int start = OCC::Time::GetTicks();
  ldr.loadFile(f->getPathName());
  int end = OCC::Time::GetTicks();
  printf("Time: %d\n", end - start);
  
  PolyMesh *dmesh;
  uvmesh = (ETexMesh*) ldr.getFirstResource (Class(TexMesh));
  dmesh = (PolyMesh*)ldr.getFirstResource(Class(PolyMesh));
  zekko = (PolyMeshActor*) ldr.getFirstObject (Class(PolyMeshActor));
  if (dmesh == NULL) return EXIT_FAILURE;
  if (uvmesh == NULL) return EXIT_FAILURE;
  if (zekko == NULL) return EXIT_FAILURE;
  
  //Check if UV mesh type correct
  printf ("uvmesh = %s\n", StringOf (ClassOf (zekko->getTexMesh())));
  
  printf ("uvmesh %s ETexMesh\n",
          SafeCast (ETexMesh, zekko->getTexMesh()) ?
          "IS" : "is NOT");
  
  printf ("uvmesh %s TexMesh\n",
          SafeCast (TexMesh, zekko->getTexMesh()) ?
          "IS" : "is NOT");
  
  printf ("uvmesh %s Resource\n",
          SafeCast (Resource, zekko->getTexMesh()) ?
          "IS" : "is NOT");
  
  printf ("uvmesh %s PolyMesh\n",
          SafeCast (PolyMesh, zekko->getTexMesh()) ?
          "IS" : "is NOT");
  
  //Setup camera
  cam3D.translate(0,35,100);
  cam3D.setCenter(center);
  //cam3D.setNearClipPlane(100.0f);
  //cam3D.setFarClipPlane(100000.0f);
  
  //Find model center
  findCenter();
  
  //Set half bunny green
  for (PolyMesh::FaceIter f(dmesh); !f.end(); ++f) {
    //f->smoothGroups = 0x1;
    if (f->firstHedge()->dstVertex()->point.y > center.y)
      dmesh->setMaterialID( *f, 1 ); }

  //Convert to static mesh
  //dmesh->updateNormals();
  smesh.fromPoly( dmesh, uvmesh );
  //zekko->setStatic(&smesh);
  
  //Load texture image
  imgDiff.readFile( "texture.jpg", "" );
  
  //Create texture from image
  texDiff = new Texture;
  texDiff->fromImage( &imgDiff );
  
  //Load specularity image
  Image imgSpec;
  imgSpec.readFile( "specularity.jpg", "" );

  //Create specularity texture
  texSpec = new Texture();
  texSpec->fromImage(&imgSpec);
  
  //Create material using texture
  PhongMaterial mat;
  mat.setDiffuseTexture(texDiff);
  //mat.setSpecularityTexture(texSpec);
  mat.setDiffuseColor(Vector3(1,0.0f,0.0f));
  mat.setAmbientColor(Vector3(0.2f,0.2f,0.2f));
  mat.setSpecularity(1.0f);
  mat.setGlossiness(0.2f);
  
  PhongMaterial mat2;
  mat2.setDiffuseTexture(texDiff);
  //mat2.setSpecularityTexture(texSpec);
  mat2.setDiffuseColor(Vector3(0.0f, 1.0f, 0.0f));
  mat2.setAmbientColor(Vector3(0.2f, 0.2f, 0.2f));
  mat2.setSpecularity(1.0f);
  mat2.setGlossiness(0.2f);
  
  MultiMaterial mm;
  mm.setNumSubMaterials(2);
  mm.setSubMaterial(0, &mat);
  mm.setSubMaterial(1, &mat2);
  zekko->setMaterial(&mm);
  
  lblFps.setLocation(Vector2(0.0f,(Float)resY));
  lblFps.setColor(Vector3(1.0f,1.0f,1.0f));
  
  //Run application
  atexit(cleanup);
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
