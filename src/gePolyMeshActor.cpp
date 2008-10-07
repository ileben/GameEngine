#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (PolyMeshActor);
  
  PolyMeshActor::PolyMeshActor()
  {
    texMesh = NULL;
    polyMesh = NULL;
  }

  PolyMeshActor::~PolyMeshActor()
  {
    if (texMesh != NULL)
      texMesh->dereference();
    if (polyMesh != NULL)
      polyMesh->dereference();
  }

  void PolyMeshActor::setTexMesh (TexMesh *newMesh)
  {
    if (texMesh != NULL)
      texMesh->dereference();

    texMesh = newMesh;
    texMesh->reference();
  }

  TexMesh* PolyMeshActor::getTexMesh ()
  {
    return texMesh;
  }

  void PolyMeshActor::setMesh (PolyMesh *newMesh)
  {
    if (polyMesh != NULL)
      polyMesh->dereference();

    polyMesh = newMesh;
    polyMesh->reference();
  }

  PolyMesh* PolyMeshActor::getMesh ()
  {
    return polyMesh;
  }

  void PolyMeshActor::render (MaterialId materialId)
  {
    if (polyMesh != NULL)
      renderMesh (materialId);
  }

  void PolyMeshActor::renderMesh (MaterialId matid)
  {
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    
    TexMesh::FaceIter uf (texMesh);
    for (PolyMesh::FaceIter f(polyMesh); !f.end(); ++f, ++uf) {
      
      //Check if this face belongs to current material
      if (f->materialId() != matid && matid != GE_ANY_MATERIAL_ID)
        continue;
      
      glBegin(GL_POLYGON);
      
      TexMesh::FaceVertIter uv(*uf);
      for(PolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
        
        //Vertex normal
        glNormal3fv ((Float*)&h->vertexNormal()->coord);
        
        //UV coordinates
        if (!uv.end()) {
          glTexCoord2f (uv->point.x, uv->point.y); }
        
        //Vertex coordinate
        glVertex3fv ((Float*)&h->dstVertex()->point);
      }
      
      glEnd();
    }

    /*
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    for (DMesh::EdgeIter e(mesh); !e.end(); ++e)
    {
      if (e->hedge1()->parentFace() != NULL && e->hedge1()->tag.id == 0 ||
          e->hedge2()->parentFace() != NULL && e->hedge2()->tag.id == 0)
          glColor3f(1.0f, 0.0f, 0.0f);
      else //continue;
        glColor3f(1.0f, 1.0f, 1.0f);

      glVertex3fv((Float*)&e->vertex1()->point);
      glVertex3fv((Float*)&e->vertex2()->point);
    }

    glEnd(); */

    
    /* //Color internal edges green, border edges red
      glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for (DynamicMesh::EdgeIter e=mesh->edges.begin(); e!=mesh->edges.end(); ++e)
      {
      if ((*e)->hedge->face == NULL || (*e)->hedge->twin->face == NULL)
        glColor3f(1,0,0);
      else
        glColor3f(0,1,0);
      
      glVertex3fv((Float*)&(*e)->vertex1()->point);
      glVertex3fv((Float*)&(*e)->vertex2()->point);
    }*/

    //Draw normals
    /*
    glColor3f(1,0,0);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for (DynamicMesh::FaceIter f(mesh); !f.end(); ++f)
    {
      Vector3 top = (*f)->center + (*f)->normal;
      glVertex3fv((Float*)&(*f)->center);
      glVertex3fv((Float*)&top);
    }

    glEnd(); */
  }
}
