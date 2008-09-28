#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Shape);
  
  Shape::Shape()
  {
    uvMesh = NULL;
    statMesh = NULL;
    dynMesh = NULL;
    useDynamic = true;
  }

  Shape::~Shape()
  {
    if (uvMesh != NULL)
      uvMesh->dereference();
    if (statMesh != NULL)
      statMesh->dereference();
    if (dynMesh != NULL)
      dynMesh->dereference();
  }

  void Shape::setUV(UMesh *mesh)
  {
    if (uvMesh != NULL)
      uvMesh->dereference();

    uvMesh = mesh;
    uvMesh->reference();
  }

  UMesh* Shape::getUV() {
    return uvMesh;
  }

  void Shape::setStatic(SMesh *mesh)
  {
    if (statMesh != NULL)
      statMesh->dereference();

    statMesh = mesh;
    statMesh->reference();
    useDynamic = false;
  }

  SMesh* Shape::getStatic() {
    return statMesh;
  }

  void Shape::setDynamic(DMesh *mesh)
  {
    if (dynMesh != NULL)
      dynMesh->dereference();

    dynMesh = mesh;
    dynMesh->reference();
    useDynamic = true;
  }

  DMesh* Shape::getDynamic() {
    return dynMesh;
  }

  void Shape::render (MaterialId materialId)
  {
    //Check if mesh set
    if (useDynamic) {
      if (dynMesh == NULL) return;
    }else if (statMesh == NULL) return;

    //Draw proper mesh
    if (useDynamic)
      renderDynamic (materialId);
    else renderStatic (materialId);
    //drawStatMesh (shape->statMesh, 0, shape->statMesh->indices.size());

  }

  void Shape::renderStatic (MaterialId materialId)
  {
    //Walk material index groups
    for (LinkedList<SMesh::IndexGroup>::Iterator
         g = statMesh->groups.begin ();
         g != statMesh->groups.end (); ++g)
    {
      //Check if the material id matches
      if (g->materialId == materialId || materialId == GE_ANY_MATERIAL_ID)
      {
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        
        glInterleavedArrays (GL_T2F_N3F_V3F, 0, statMesh->data.buffer());
        glDrawElements (GL_TRIANGLES, g->count, GL_UNSIGNED_INT,
                        statMesh->indices.buffer() + g->start);
        
        glDisableClientState (GL_VERTEX_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);

        break;
      }
    }
  }

  void Shape::renderDynamic (MaterialId matid)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* //TODO: Remove when sure that no mesh file can break HMesh
    for (int pass=1; pass<=2; ++pass) {
    
      if (pass == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_COLOR_MATERIAL);
        //glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

      }else if (pass == 2) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
      } */

    
    UMesh::FaceIter uf(uvMesh);
    for (DMesh::FaceIter f(dynMesh); !f.end(); ++f, ++uf) {
      
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
      for(DMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
        
        //Interpolate per-vertex normals in smooth mode
        if (dynMesh->getShadingModel() == SHADING_SMOOTH)
          glNormal3fv ((Float*)&h->smoothNormal()->coord);
        
        //UV coordinates
        if (!uv.end()) {
          glTexCoord2f (uv->point.x, uv->point.y); }
        
        //Vertex coordinate
        glVertex3fv ((Float*)&h->dstVertex()->point);
      }
      
      glEnd();
    }
    
    //}

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
    }*/
    

    glEnd();
  }
}
