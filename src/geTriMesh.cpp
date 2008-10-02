#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;


namespace GE
{
  DEFINE_CLASS (TriMesh);
  
  
  struct UniqueVertex
  {
    StaticId staticId;
    MaterialId materialId;
    void* uvertexPtr;
    void* snormalPtr;
  };

  /*-------------------------------------------------
  When packing a dynamic editable mesh into a static
  representation, the final memory layout of data
  has to be appropriate for OpenGL rendering calls.
  Since glDrawElements doesn't take separate indices
  for points, texture coordinates and normals, we
  have to produce multiple vertex variants to
  represent the same dynamic vertex in many of its
  incident faces.
  
  The algorithm walks all the incident faces of each
  vertex and checks whether the normal or texture
  coordinate differ for any of the faces. The
  uniqueness of the smooth normal is already given
  via the 'normalGroup' id produced by the normal
  calculation algorithm. Additionally, in the first
  pass the pointers to the respective vertices in
  the dynamic UV mesh are copied into vert-per-face
  halfedges' algorithm helper tags.
  --------------------------------------------------*/

  void TriMesh::fromPoly (PolyMesh *m, TexMesh *um)
  {
    PolyMesh::FaceIter f;
    PolyMesh::FaceHedgeIter h;
    TexMesh::FaceIter uf;
    TexMesh::FaceHedgeIter uh;
    ArrayList<UniqueVertex> uniqVerts;
    StaticId nextStaticId = 0;

    data.clear();
    indices.clear();


    //Store UV pointers into vert-per-face hedges
    for (f.begin(m), uf.begin(um); !f.end(); ++f, ++uf) {
      for (h.begin(*f), uh.begin(*uf); !h.end(); ++h, ++uh) {
        if (!uh.end()) h->tag.ptr = uh->dstVertex();
        else h->tag.ptr = NULL;
      }}

    //Walk all the vertices of the mesh
    for (PolyMesh::VertIter v(m); !v.end(); ++v) {

      //An array for unique vertices
      uniqVerts.clear();

      //Walk adjacent faces and output unique vertex variants
      for (PolyMesh::VertFaceIter vf(*v); !vf.end(); ++vf) {
        PolyMesh::HalfEdge *vfh = vf.hedgeToVertex();

        //Walk existing vertex variants
        bool existingFound = false;
        for (int u=uniqVerts.size()-1; u>=0; --u) {

          //Check for match
          if (uniqVerts[u].materialId == vf->materialId() &&
              uniqVerts[u].uvertexPtr == vfh->tag.ptr &&
              uniqVerts[u].snormalPtr == vfh->smoothNormal()) {

              //Target existing one
              vfh->tag.id = uniqVerts[u].staticId;
              existingFound = true;
              break;
          }}

        //Output a new vertex variant
        if (!existingFound) {

          //Uniqueness info
          UniqueVertex uniq;
          uniq.materialId = vf->materialId();
          uniq.uvertexPtr = vfh->tag.ptr;
          uniq.snormalPtr = vfh->smoothNormal();
          uniq.staticId = nextStaticId;
          uniqVerts.pushBack(uniq);

          //Add UV coord to data
          if (vfh->tag.ptr == NULL) {
            data.pushBack(0.0f);
            data.pushBack(0.0f);
          }else{
            TexMesh::Vertex *uv = (TexMesh::Vertex*)vfh->tag.ptr;
            data.pushBack(uv->point.x);
            data.pushBack(uv->point.y); }
          
          //Add normal coord to data
          data.pushBack(vfh->smoothNormal()->coord.x);
          data.pushBack(vfh->smoothNormal()->coord.y);
          data.pushBack(vfh->smoothNormal()->coord.z);
          
          //Add vertex coord to data
          data.pushBack(v->point.x);
          data.pushBack(v->point.y);
          data.pushBack(v->point.z);

          //Assign static id
          vfh->tag.id = nextStaticId++;
        }
       
      }//Walk adjacent faces
    }//Walk all vertices

    
    //Walk all the faces of the mesh
    for (LinkedList<MaterialId>::Iterator mid=m->materialsUsed.begin();
         mid != m->materialsUsed.end(); ++mid) {
      
      //Initialize material index group
      IndexGroup &grp = *groups.pushBack(IndexGroup());
      grp.materialId = *mid;
      grp.start = indices.size();
      grp.count = 0;
      
      //Walk faces of current material
      for (PolyMesh::MaterialFaceIter mf(m, *mid); !mf.end(); ++mf) {
        
        PolyMesh::HalfEdge *cur = mf->firstHedge();
        PolyMesh::HalfEdge *prev = cur->prevHedge();
        PolyMesh::HalfEdge *next = cur->nextHedge();
        
        do { //Triangulate face
          
          indices.pushBack(prev->tag.id);
          indices.pushBack(cur->tag.id);
          indices.pushBack(next->tag.id);
          grp.count += 3;
          cur = cur->nextHedge();
          next = next->nextHedge();
          
        } while (next != prev);
      }
    }
    
    if (Kernel::Instance->hasRangeElements) {
      printf("Max verts: %d\n", Kernel::Instance->maxElementsVertices);
      printf("Max indices: %d\n", Kernel::Instance->maxElementsIndices);
    }
  }

}//namespace GE
