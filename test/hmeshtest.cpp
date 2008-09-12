#include <stdio.h>
#include <vector>
using namespace std;

#include <geEngine.h>
using namespace GE;

#if defined(__APPLE__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

class MyMesh : public HMesh {
public:

  class Vertex; class HalfEdge; class Edge; class Face;
  
  class Vertex : public VertexBase <MyMesh,HMesh> {
    REGISTER_CLASS(Vertex); public:
    bool selected;
    Vector2 point;
    Vertex() : selected(false) {}
  };

  class Face : public FaceBase <MyMesh,HMesh>{
    REGISTER_CLASS(Face); public:
    Vector2 center;
    bool selected;
    Face() : selected(false) {}
  };
  
  class HalfEdge : public HalfEdgeBase <MyMesh,HMesh> {
    REGISTER_CLASS(HalfEdge)};
  
  class Edge : public EdgeBase <MyMesh,HMesh> {
    REGISTER_CLASS(Edge)};

  MyMesh() {
    setClasses(
      Class(Vertex),
      Class(HalfEdge),
      Class(Edge),
      Class(Face)); }
  
#include "geHmeshAdjiter.h"
#include "geHmeshDataiter.h"
};


MyMesh mesh;
list<MyMesh::Vertex*> selection;
list<MyMesh::Face*> selectionF;


void reshape(int w, int h)
{
  glViewport(0,0,w,h);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,w,h,0);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display()
{
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  
  //draw edges
  glBegin(GL_LINES);
  
  for (MyMesh::HedgeIter e(&mesh); !e.end(); ++e) {
    
    //get both end-points
    MyMesh::HalfEdge *e1 = *e;
    MyMesh::HalfEdge *e2 = e1->twinHedge();
    Vector2 v1 = e1->dstVertex()->point;
    Vector2 v2 = e2->dstVertex()->point;
    
    if (e1->face != NULL) {
      
      //offset points towards face center
      MyMesh::Face *f = e1->parentFace();
      Vector2 vc1 = (f->center - v1).normalize();
      Vector2 vc2 = (f->center - v2).normalize();
      v1.offsetV(vc1, 10);
      v2.offsetV(vc2, 10);
      glColor3f(0,1,0);
      
    }else{
      glColor3f(1,0,0);
    }

    //offset points to shorten ends
    Vector2 ve1 = (v1 - v2).normalize();
    Vector2 ve2 = (v2 - v1).normalize();
    v2.offsetV(ve1, 10);
    v1.offsetV(ve2, 10);
    
    //draw edge
    glVertex2fv((Float*)&v1);
    glVertex2fv((Float*)&v2);
    
    //draw direction marker
    Vector2 T(-ve2.y, ve2.x);
    Vector2 dm = v1 + ve2*15 + T*4;
    glVertex2fv((Float*)&v1);
    glVertex2fv((Float*)&dm);

    //draw link from this end to next start
    Vector2 ne = e1->nextHedge()->dstVertex()->point;
    Vector2 ns = e1->nextHedge()->twinHedge()->dstVertex()->point;
    Vector2 ven = (ne - ns).normalize();
    if (e1->nextHedge()->parentFace() != NULL) {
      MyMesh::Face *face = e1->nextHedge()->parentFace();
      Vector2 vcn = (face->center - ns).normalize();
      ns.offsetV(vcn,10);
    }
    ns.offsetV(ven, 10);
    glColor3f(0.4,0.6,1);
    glVertex2fv((Float*)&v1);
    glVertex2fv((Float*)&ns);
  }
  
  glEnd();
  
  //draw face centers
  glPointSize(4);
  glBegin(GL_POINTS);
  for (MyMesh::FaceIter f(&mesh); !f.end(); ++f) {

    if (f->selected)
      glColor3f(1,1,1);
    else glColor3f(0,1,0);

    glVertex2fv((Float*)&f->center);
  }
  glEnd();
  
  //draw vertice
  glBegin(GL_POINTS);
  for (MyMesh::VertIter v(&mesh); !v.end(); ++v) {
    
    if (v->selected) {
      glPointSize(10);
      glColor3f(1,1,1);
    }else{
      glPointSize(4);
      glColor3f(1,0,0);
    }
    glVertex2fv((Float*)&v->point);
  }
  glEnd();
  
  
  glFlush();
}

void selectVertex(MyMesh::Vertex *vert)
{
  vert->selected = !vert->selected;
  
  if (vert->selected)
    selection.push_back(vert);
  else
    selection.remove(vert);
  
  printf("selected ID: %i\n", vert->tag.id);
}

void selectFace(MyMesh::Face *face)
{
  face->selected = !face->selected;
  
  if (face->selected)
    selectionF.push_back(face);
  else
    selectionF.remove(face);
}

void deselectAll()
{
  while (selection.size() > 0) {
    selection.back()->selected = false;
    selection.pop_back();
  }
  
  while (selectionF.size() > 0) {
    selectionF.back()->selected = false;
    selectionF.pop_back();
  }
}

void calculateCenter(MyMesh::Face *face)
{
  int pointcount = 0;
  face->center.set(0,0);
  
  for (MyMesh::FaceVertIter v(face); !v.end(); ++v) {
    face->center += v->point;
    pointcount++;
  }
  
  face->center /= pointcount;
}

void calculateCenters()
{
  for (MyMesh::FaceIter f(&mesh); !f.end(); ++f)
    calculateCenter(*f);
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 27://escape
    
    //exit on escape
    exit(0);
    
    break;
  case 13://enter
  
    //we need at least 3 points for a face
    if (selection.size() >= 3) {
  
      int facecount = 0;
      
      //copy selected vertices from list into an array
      MyMesh::Vertex **faceVerts = new MyMesh::Vertex*[selection.size()];
      for (list<MyMesh::Vertex*>::iterator v = selection.begin();
           v!=selection.end(); v++) faceVerts[facecount++] = *v;
      
      //create new face from seleted vertices
      MyMesh::Face *newFace = (MyMesh::Face*)mesh.addFace(
        (HMesh::Vertex**)&faceVerts[0],facecount);
      if (newFace == NULL) {
        printf("ADD FACE failed! Invalid vertex list!\n"); break;}
      calculateCenter(newFace);
    }
    
    break;
  case 127:{//backspace
    
    //check for face removal first
    if (!selectionF.empty()) {
      MyMesh::Face *face = selectionF.front();
      deselectAll();
      mesh.removeFace(face);
      mesh.clearInvalid();
    }
    
    //we need 2 points for an edge
    if (selection.size() != 2) break;
    
    //pick two verts from the selection
    MyMesh::Vertex* vert1 = selection.front();
    MyMesh::Vertex* vert2 = selection.back();
    deselectAll();
    
    //find edge to be removed
    MyMesh::HalfEdge *h = vert1->outHedgeTo(vert2);
    if (h == NULL) { printf("REMOVE EDGE failed! Invalid vertex list!\n"); break; }
    mesh.removeEdge(h->edge);
    mesh.clearInvalid();
    calculateCenters();
    
    break;}
  case 32://space
    
    //we need 2 points for an edge
    if (selection.size() != 2) break;
    
    //pick two verts from the selection
    MyMesh::Vertex* vert1 = selection.front();
    MyMesh::Vertex* vert2 = selection.back();
    deselectAll();
    
    //weld and free unneeded memory
    if (!mesh.weldVertices(vert1, vert2))
      printf("WELD returned false!\n");
    /*
    MyMesh::HalfEdge *he = vert1->edgeTo(vert2);
    if (he == NULL) return;
    if (!mesh.collapseEdge(he->edge))
      printf("collapseEdge() returned FALSE!\n");*/
    mesh.clearInvalid();
    calculateCenters();
    
    break;
  }
  
  glutPostRedisplay();
}

void specialKey(int key, int x, int y)
{
  switch (key)
  {
  case 101:
    
    for (list<MyMesh::Vertex*>::iterator v=selection.begin();
         v!=selection.end(); ++v) (*v)->point.y -= 5;
    
    break;
  case 103:
    
    for (list<MyMesh::Vertex*>::iterator v=selection.begin();
         v!=selection.end(); ++v) (*v)->point.y += 5;
    
    break;
  case 100:
    
    for (list<MyMesh::Vertex*>::iterator v=selection.begin();
         v!=selection.end(); ++v) (*v)->point.x -= 5;
    
    break;
  case 102:
    
    for (list<MyMesh::Vertex*>::iterator v=selection.begin();
         v!=selection.end(); ++v) (*v)->point.x += 5;
    
    break;
  }
  
  calculateCenters();
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  if (state != GLUT_DOWN)
    return;
  
  int mods = glutGetModifiers();

  if (button == GLUT_RIGHT_BUTTON || mods & GLUT_ACTIVE_CTRL) {
    //deselect on RCLICK
    deselectAll();
    glutPostRedisplay();
    return;
  }
  
  if (mods & GLUT_ACTIVE_SHIFT) {
    
    //add new vertex on SHIFT + LCLICK
    MyMesh::Vertex *newVert = (MyMesh::Vertex*)mesh.addVertex();
    newVert->tag.id = mesh.verts.size();
    newVert->point = Vector2(x,y);
    selectVertex(newVert);
    
  }else{
    
    //find vertex to select on LCLICK
    for (MyMesh::VertIter v(&mesh); !v.end(); ++v) {
      
      Vector2 vdist = Vector2(x,y) - v->point;
      float dist = vdist.norm();
      
      if (dist < 10) {
        selectVertex(*v);
        glutPostRedisplay();
        return;
      }
    }
    
    //find face to select on LCLICK
    for (MyMesh::FaceIter f(&mesh); !f.end(); ++f) {
      
      Vector2 vdist = Vector2(x,y) - f->center;
      float dist = vdist.norm();
      
      if (dist < 10) {
        selectFace(*f);
        glutPostRedisplay();
        return;
      }
    }
  }

  glutPostRedisplay();
}

vector<MyMesh::Vertex*> verts;
void addVert(float x, float y)
{
  MyMesh::Vertex *newVert = (MyMesh::Vertex*)mesh.addVertex();
  newVert->tag.id = mesh.verts.size();
  newVert->point = Vector2(x,y);
  verts.push_back(newVert);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  
  glutInitWindowPosition(0,0);
  glutInitWindowSize(800,600);
  glutCreateWindow("HMesh Test");
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKey);
  glutMouseFunc(mouse);
  
  addVert(100,100);
  addVert(50,150);
  addVert(100,200);
  addVert(150,150);
  
  addVert(100,250);
  addVert(50,300);
  addVert(100,350);
  addVert(150,300);
  
  MyMesh::Vertex *tri[3];
  tri[0] = verts[0];
  tri[1] = verts[1];
  tri[2] = verts[3];
  mesh.addFace((HMesh::Vertex**)tri,3);
  tri[0] = verts[1];
  tri[1] = verts[2];
  tri[2] = verts[3];
  mesh.addFace((HMesh::Vertex**)tri,3);
  tri[0] = verts[4];
  tri[1] = verts[5];
  tri[2] = verts[7];
  mesh.addFace((HMesh::Vertex**)tri,3);
  tri[0] = verts[5];
  tri[1] = verts[6];
  tri[2] = verts[7];
  mesh.addFace((HMesh::Vertex**)tri,3);
  calculateCenters();
  
  //void *meshdata = mesh.serialize();
  //mesh.clear();
  //mesh.deserialize(meshdata);
  //free(meshdata);
  calculateCenters();
  verts.clear();
  
  /*
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      
      Vector2 p(100 + x*100, 100 + y*100);
      MyMesh::Vertex *newVert = mesh.addVertex(p);
      newVert->id = mesh.verts.size();
      verts.push_back(newVert);
    }
  }
  
  for (int y=0; y<3; y++) {
    for (int x=0; x<3; x++) {
      
      MyMesh::Vertex *face[3];
      face[0] = verts[y*4 + x];
      face[1] = verts[(y+1)*4 + x];
      face[2] = verts[(y+1)*4 + (x+1)];
      MyMesh::Face *f = mesh.addFace(face, 3);
      calculateCenter(f);
      
      face[0] = verts[y*4 + x];
      face[1] = verts[(y+1)*4 + (x+1)];
      face[2] = verts[y*4 + (x+1)];
      f = mesh.addFace(face, 3);
      calculateCenter(f);
    }
  }*/
  
  glutMainLoop();
}
