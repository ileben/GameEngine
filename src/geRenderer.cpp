#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;;

namespace GE
{
  DEFINE_CLASS (Renderer);
  
  Renderer::Renderer()
  {
    camera = NULL;
    viewX = 0;
    viewY = 0;
    viewW = 0;
    viewH = 0;
  }

  void Renderer::setBackColor(const Vector3 &color)
  {
    back = color;
  }

  /*----------------------------------------------------
   * Changes the target rendering area on the GL window
   *----------------------------------------------------*/

  void Renderer::setViewport(int x, int y, int width, int height)
  {
    viewX = x;
    viewY = y;
    viewW = width;
    viewH = height;
    glViewport(x, y, width, height);

    if (camera != NULL)
      camera->updateProjection(viewW, viewH);
  }

  /*-----------------------------------------
   * Changes the camera used for rendering
   *-----------------------------------------*/

  void Renderer::setCamera(Camera *camera)
  {
    this->camera = camera;
    camera->updateProjection(viewW, viewH);
    camera->updateView();
  }

  Camera* Renderer::getCamera() {
    return camera;
  }

  /*--------------------------------------
   * Initializes next frame for rendering
   *--------------------------------------*/

  void Renderer::begin()
  {
    glClearColor (back.x, back.y, back.z, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (camera != NULL) camera->updateView ();
  }

  void Renderer::end()
  {
    glutSwapBuffers ();
  }

  /*---------------------------------------------
   * Renders geometry defined by a dynamic mesh
   * according to given type of drawing
   *---------------------------------------------*/

  void Renderer::drawDynMesh(DMesh *mesh, UMesh *umesh, int matid)
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

    
    UMesh::FaceIter uf(umesh);
    for (DMesh::FaceIter f(mesh); !f.end(); ++f, ++uf) {
      
      //Check if this face belongs to current material
      if (f->materialId() != (MaterialId)matid && matid != -1)
        continue;
      
      glBegin(GL_POLYGON);
      
      //Use face normal for all vertices in flat mode
      if(mesh->getShadingModel() == SHADING_FLAT)
        glNormal3fv((Float*)&f->normal);
      
      /* //TODO: Remove when sure that no mesh file can break HMesh
      glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
      for(DMesh::FaceHedgeIter fh(*f); !fh.end(); ++fh)
        if (fh->tag.id == 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f);*/
      
      UMesh::FaceVertIter uv(*uf);
      for(DMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
        
        //Interpolate per-vertex normals in smooth mode
        if (mesh->getShadingModel() == SHADING_SMOOTH)
          glNormal3fv((Float*)&h->smoothNormal()->coord);
        
        //UV coordinates
        if (!uv.end()) {
          glTexCoord2f(uv->point.x, uv->point.y); }
        
        //Vertex coordinate
        glVertex3fv((Float*)&h->dstVertex()->point);
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


  /*--------------------------------------------
   * Renders geometry defined by a highly
   * optimized static mesh. Here type of
   * drawing is fixed by the mesh setup so
   * switching from filled to wireframe is not
   * possible on-the-fly.
   *--------------------------------------------*/

  void Renderer::drawStatMesh(SMesh *mesh, StaticId start, StaticId count)
  {
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    
    glInterleavedArrays (GL_T2F_N3F_V3F, 0, mesh->data.buffer());
    glDrawElements (GL_TRIANGLES, count, GL_UNSIGNED_INT,
                    mesh->indices.buffer() + start);
    
    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
  }


  /*-------------------------------------------
   * Renders a shape geometry with appearance
   * defined by its material and transformed
   * by current transformation matrix.
   *-------------------------------------------*/

  void Renderer::drawShape(Shape *shape)
  {
    //Check if mesh set
    if (shape->useDynamic) {
      if (shape->dynMesh == NULL) return;
    }else if (shape->statMesh == NULL) return;
    
    
    //Check material type
    if (shape->material == NULL) {

      Material::BeginDefault ();

      
    }else if (ClassOf (shape->material) == Class (StandardMaterial)) {
      
      //Setup material GL state
      shape->material->begin ();
      
      //Draw proper mesh
      if (shape->useDynamic)
        drawDynMesh (shape->dynMesh, shape->uvMesh);  
      else drawStatMesh (shape->statMesh, 0, shape->statMesh->indices.size());
      
      shape->material->end ();
      
      
    }else if (ClassOf (shape->material) == Class (MultiMaterial)) {
      
      MultiMaterial *material = (MultiMaterial*) shape->material;
      
      if (shape->useDynamic) {
        
        //Walk list of materials used by this mesh
        for (LinkedList<MaterialId>::Iterator m =
             shape->dynMesh->materialsUsed.begin ();
             m != shape->dynMesh->materialsUsed.end (); ++m)
        {  
          //Setup proper GL state by material id
          material->selectSubMaterial (*m);
          material->begin ();
          
          //Draw dynamic mesh
          drawDynMesh (shape->dynMesh, shape->uvMesh, (int)*m);
          
          material->end ();
        }
        
      }else{
        
        //Walk material index groups
        for (LinkedList<SMesh::IndexGroup>::Iterator g =
             shape->statMesh->groups.begin ();
             g != shape->statMesh->groups.end (); ++g)
        {
          //Setup proper GL state by material id
          material->selectSubMaterial (g->materialId);
          material->begin ();
          
          //Draw static mesh
          drawStatMesh (shape->statMesh, g->start, g->count);
          
          material->end ();
        }
      }
    }
  }
  
  void Renderer::drawWidget(Widget *w)
  {
    GLProgram::UseFixed ();
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_CULL_FACE);
    glDisable (GL_LIGHTING);
    glDisable (GL_BLEND);
    
    glEnable (GL_COLOR_MATERIAL);
    
    w->draw ();
  }

}/* namespace GE */
