#include "geHmesh.h"

namespace GE
{
  typedef HMesh::Vertex         Vertex;
  typedef HMesh::HalfEdge       HalfEdge;
  typedef HMesh::Edge           Edge;
  typedef HMesh::Face           Face;
  typedef HMesh::VertHedgeIter  VertHedgeIter;
  typedef HMesh::VertVertIter   VertVertIter;

  /*
  -----------------------------------------------------
  Vertex
  -----------------------------------------------------*/


  HalfEdge* Vertex::outHedgeTo (Vertex *v)
  {
    for (VertHedgeIter e((Vertex*)this); !e.end(); ++e)
      if ((*e)->vert == v)
        return (*e);
    return false;
  }

  bool Vertex::isConnectedTo (Vertex *v)
  {
    return outHedgeTo( v ) != NULL;
  }

  Face* Vertex::commonFaceTo (Vertex *v)
  {
    for (HMesh::VertFaceIter vf1(this); !vf1.end(); ++vf1)
      for (HMesh::VertFaceIter vf2(v); !vf2.end(); ++vf2)
        if (*vf1 == *vf2)
          return *vf1;

    return NULL;
  }

  int Vertex::degree()
  {
    int deg = 0;
    for (VertVertIter v(this); !v.end(); ++v)
      deg++;    
    return deg;
  }

  /*
  -----------------------------------------------------
  HalfEdge
  -----------------------------------------------------*/

  int HalfEdge::loopLength()
  {
    int count = 0;
    HalfEdge *he = this; do {
      he = he->next; count++;
    }while(he != this);
    
    return count;
  }

  bool HalfEdge::isLoopTriangle() {
    return next->next->next == this;
  }

  bool HalfEdge::isLoopEqual(HalfEdge *l)
  {
    //it suffices to find an equal hedge in a loop
    //to confirm the loops are equal since they
    //follow the same path
    HalfEdge *he = this; do {
      if (he == l) return true;
      he = he->next;
    }while(he != this);
    
    return false;
  }

  Vertex* HalfEdge::oppositeVertex() {
    //assume triangle
    return next->vert;
  }

  /*
  -----------------------------------------------------
  Face
  -----------------------------------------------------*/

  HalfEdge* Face::hedgeTo(Vertex* v)
  {
    //Find hedge that points to given vertex
    HalfEdge *he = hedge; do {
      if (he->vert == v) return he;
      he = he->next;
    }while(he != hedge);

    return NULL;
  }

  int Face::vertexCount() {
    return hedge->loopLength();
  }

  bool Face::isTriangle() {
    return hedge->isLoopTriangle();
  }

  /*
  ----------------------------------------------
  Serialized mesh entity data types. Note that
  these don't use traits so all the custom data
  is lost when serializing.
  ----------------------------------------------*/

  struct VertexS {
    //Point point;
    int hedge;
  };
  
  struct HalfEdgeS {
    int twin;
    int next;
    int prev;
    int vert;
    int edge;
    int face;
  };
  
  struct EdgeS {
    int hedge;
  };
  
  struct FaceS {
    int hedge;
  };

  /*
  ------------------------------------------------------
  Sets classes to be used when entities are constructed
  ------------------------------------------------------*/

  void HMesh::setClasses (Class cnVertex, Class cnHedge,
                          Class cnEdge, Class cnFace)
  {
    classVertex = cnVertex;
    classHalfEdge = cnHedge;
    classEdge = cnEdge;
    classFace = cnFace;
  }

  /*
  ---------------------------------------------
  Entity insertion and removal functions.
  These make sure that entities have proper
  handles to list nodes for removal and
  store them into invalidation lists when
  removed by mesh operations.
  ---------------------------------------------*/

  void HMesh::insertVert(Vertex *v) {
    v->handle = verts.pushBack(v);
  }
  
  void HMesh::insertHalfEdge(HalfEdge *he) {
    he->handle = hedges.pushBack(he);
  }
  
  void HMesh::insertEdge(Edge *e) {
    e->handle = edges.pushBack(e);
  }
  
  void HMesh::insertFace(Face *f) {
    f->handle = faces.pushBack(f);
  }
  
  ListHandle HMesh::deleteVert(Vertex *v) {
    v->valid = false;
    invalid_verts.pushBack(v);
    return verts.removeAt(v->handle);
  }
  
  ListHandle HMesh::deleteHalfEdge(HalfEdge *he) {
    he->valid = false;
    invalid_hedges.pushBack(he);
    return hedges.removeAt(he->handle);
  }
  
  ListHandle HMesh::deleteEdge(Edge *e) {
    e->valid = false;
    invalid_edges.pushBack(e);
    return edges.removeAt(e->handle);
  }
  
  ListHandle HMesh::deleteFace(Face *f) {
    f->valid = false;
    invalid_faces.pushBack(f);
    return faces.removeAt(f->handle);
  }
  
  void HMesh::deleteEdgeWhole(Edge *e) {
    e->valid = false;
    e->hedge->valid = false;
    e->hedge->twin->valid = false;
    edges.removeAt(e->handle);
    hedges.removeAt(e->hedge->handle);
    hedges.removeAt(e->hedge->twin->handle);
    invalid_edges.pushBack(e);
    invalid_hedges.pushBack(e->hedge);
    invalid_hedges.pushBack(e->hedge->twin);
  }

  /*
  -----------------------------------------------
  Finally deletes the invalidated mesh entities
  which were removed by mesh operations
  -----------------------------------------------*/

  void HMesh::clearInvalid()
  {
    for (ListHandle v=invalid_verts.begin();
        v!=invalid_verts.end(); ++v) delete (Vertex*)*v;
    
    for (ListHandle he=invalid_hedges.begin();
        he!=invalid_hedges.end(); ++he) delete (HalfEdge*)*he;
    
    for (ListHandle e=invalid_edges.begin();
        e!=invalid_edges.end(); ++e) delete (Edge*)*e;
    
    for (ListHandle f=invalid_faces.begin();
        f!=invalid_faces.end(); ++f) delete (Face*)*f;
    
    invalid_verts.clear();
    invalid_hedges.clear();
    invalid_edges.clear();
    invalid_faces.clear();
  }

  /*
  ------------------------------------------------
  Deletes all the mesh entities and destroys the
  entire mesh structure.
  ------------------------------------------------*/

  void HMesh::clear()
  {
    clearInvalid();
    
    for (ListHandle v=verts.begin();
        v!=verts.end(); ++v) delete (Vertex*)*v;
    
    for (ListHandle he=hedges.begin();
        he!=hedges.end(); ++he) delete (HalfEdge*)*he;
    
    for (ListHandle e=edges.begin();
        e!=edges.end(); ++e) delete (Edge*)*e;
    
    for (ListHandle f=faces.begin();
        f!=faces.end(); ++f) delete (Face*)*f;
    
    verts.clear();
    hedges.clear();
    edges.clear();
    faces.clear();
  }

  /*
  -------------------------------------------------------------
  Transfers the mesh entities from given HMesh to this one
  adding them to existing mesh structure. Just the pointers
  to entities are transfered without actually copying their
  data. The given mesh structured is empty after the function
  returns. This is the fastest way to merge two meshes.
  -------------------------------------------------------------*/

  void HMesh::mergeWith(HMesh *mesh)
  {
    //transfer data from [mesh] to [this] and readjust handles
    for (ListHandle v=mesh->verts.begin(); v!=mesh->verts.end(); ++v) {
      ((Vertex*)(*v))->handle = verts.pushBack(*v);
    }
    for (ListHandle h=mesh->hedges.begin(); h!=mesh->hedges.end(); ++h) {
      ((HalfEdge*)(*h))->handle = hedges.pushBack(*h);;
    }
    for (ListHandle e=mesh->edges.begin(); e!=mesh->edges.end(); ++e) {
      ((Edge*)(*e))->handle = edges.pushBack(*e);;
    }
    for (ListHandle f=mesh->faces.begin(); f!=mesh->faces.end(); ++f) {
      ((Face*)(*f))->handle = faces.pushBack(*f);
    }
    
    //clear [mesh]
    mesh->verts.clear();
    mesh->hedges.clear();
    mesh->edges.clear();
    mesh->faces.clear();
  }

  /*
  --------------------------------------------------------------
  Stores the mesh structure into a chunk of memory independent
  of its actual memory layout by swapping the pointer
  references with indexed entity id's. The resulting mesh
  representation is safe for transfer across network.
  --------------------------------------------------------------*/

  void* HMesh::serialize(int *outSize)
  {
    int id = 0;
    VertIter v(this);
    HedgeIter he(this);
    EdgeIter e(this);
    FaceIter f(this);
    
    //index data collections
    for (v.begin(this); !v.end(); ++v)
      (*v)->tag.id = id++;
    
    id = 0;
    for (he.begin(this); !he.end(); ++he)
      (*he)->tag.id = id++;
    
    id = 0;
    for (e.begin(this); !e.end(); ++e)
      (*e)->tag.id = id++;
    
    id=0;
    for (f.begin(this); !f.end(); ++f)
      (*f)->tag.id = id++;
    
    //calculate overall size and alloc memory
    int size = ( 4* sizeof(int) +
                verts.size() * sizeof(VertexS) +
                hedges.size() * sizeof(HalfEdgeS) +
                edges.size() * sizeof(EdgeS) +
                faces.size() * sizeof(FaceS) );
    
    void *data = malloc(size);
    
    //serialize collection sizes
    int *dataCount = (int*)data;
    dataCount[0] = verts.size();
    dataCount[1] = hedges.size();
    dataCount[2] = edges.size();
    dataCount[3] = faces.size();
    
    //serialize vertices
    VertexS *dataVert = (VertexS*)(&dataCount[4]);
    for (v.begin(this); !v.end(); ++v) {
      //dataVert->point = (*v)->point;
      dataVert->hedge = (*v)->hedge->tag.id;
      dataVert++;
    }
    
    //serialize half-edges
    HalfEdgeS *dataHedge = (HalfEdgeS*)dataVert;
    for (he.begin(this); !he.end(); ++he) {
      dataHedge->twin = (*he)->twin->tag.id;
      dataHedge->next = (*he)->next->tag.id;
      dataHedge->prev = (*he)->prev->tag.id;
      dataHedge->vert = (*he)->vert->tag.id;
      dataHedge->edge = (*he)->edge->tag.id;
      dataHedge->face = ((*he)->face == NULL) ? -1 : (*he)->face->tag.id;
      dataHedge++;
    }
    
    //serialize edges
    EdgeS *dataEdge = (EdgeS*)dataHedge;
    for (e.begin(this); !e.end(); ++e) {
      dataEdge->hedge = (*e)->hedge->tag.id;
      dataEdge++;
    }
    
    //serialize faces
    FaceS *dataFace = (FaceS*)dataEdge;
    for (f.begin(this); !f.end(); ++f) {
      dataFace->hedge = (*f)->hedge->tag.id;
      dataFace++;
    }
    
    //output data size if requested
    if (outSize != NULL)
      *outSize = size;
    
    return data;
  }

  /*
  -----------------------------------------------------------------
  Unpacks the mesh structure from given serialized representation
  back into pointer-referenced representation for fast operations
  on mesh. The unserialized entities are added to the existing
  mesh structure rather than replacing it.
  -----------------------------------------------------------------*/
  
  void HMesh::deserialize(void *data) 
  {
    int v, he, e, f;
    
    //unserialize collection sizes
    int *dataCount = (int*)data;
    int countVert = dataCount[0];
    int countHedge = dataCount[1];
    int countEdge = dataCount[2];
    int countFace = dataCount[3];
    
    //obtain pointers to begginning of collections
    VertexS *dataVert = (VertexS*)(&dataCount[4]);
    HalfEdgeS *dataHedge = (HalfEdgeS*)(&dataVert[countVert]);
    EdgeS *dataEdge = (EdgeS*)(&dataHedge[countHedge]);
    FaceS *dataFace = (FaceS*)(&dataEdge[countEdge]);
    
    //generate vertices
    Vertex **pointerVert = new Vertex*[countVert];
    for (v=0; v<countVert; ++v) {
      pointerVert[v] = (Vertex*)classVertex->instantiate();
      insertVert(pointerVert[v]);
    }
    
    //generate half-edges
    HalfEdge **pointerHedge = new HalfEdge*[countHedge];
    for (he=0; he<countHedge; ++he) {
      pointerHedge[he] = (HalfEdge*)classHalfEdge->instantiate();
      insertHalfEdge(pointerHedge[he]);
    }
    
    //generate edges
    Edge **pointerEdge = new Edge*[countEdge];
    for (e=0; e<countEdge; ++e) {
      pointerEdge[e] = (Edge*)classEdge->instantiate();
      insertEdge(pointerEdge[e]);
    }
    
    //generate faces
    Face **pointerFace = new Face*[countFace];
    for (f=0; f<countFace; ++f) {
      pointerFace[f] = (Face*)classFace->instantiate();
      insertFace(pointerFace[f]);
    }
    
    //unserialize vertices
    for (v=0; v<countVert; ++v) {
      //pointerVert[v]->point = dataVert[v].point;
      pointerVert[v]->hedge = pointerHedge[dataVert[v].hedge];
    }
    
    //unserialize half-edges
    for (he=0; he<countHedge; ++he) {
      pointerHedge[he]->twin = pointerHedge[dataHedge[he].twin];
      pointerHedge[he]->next = pointerHedge[dataHedge[he].next];
      pointerHedge[he]->prev = pointerHedge[dataHedge[he].prev];
      pointerHedge[he]->vert = pointerVert[dataHedge[he].vert];
      pointerHedge[he]->edge = pointerEdge[dataHedge[he].edge];
      int faceID = dataHedge[he].face;
      pointerHedge[he]->face = (faceID == -1) ?  NULL : pointerFace[faceID];
    }
    
    //unserialize edges
    for (e=0; e<countEdge; ++e)
      pointerEdge[e]->hedge = pointerHedge[dataEdge[e].hedge];
    
    //unserialize faces
    for (f=0; f<countFace; ++f)
      pointerFace[f]->hedge = pointerHedge[dataFace[f].hedge];
  }

  /*
  -----------------------------------------------------
  Helper functions that perform various sub-operations
  that make a part of greater high-level operations
  exposed to the final user.
  -----------------------------------------------------*/

  void HMesh::linkOverPerpendicular(HalfEdge *he)
  {
    //two adjacent edges to [he]'s vert and
    //perpendicular to [he] are linked
    //making the loop bypass edge [he]
    he->twin->prev->next = he->next;
    he->next->prev = he->twin->prev;
    
    //first hedge of verts and face are reset
    //in case they where set to [he]
    he->vert->hedge = he->next;
    if (he->face != NULL)
      he->face->hedge = he->next;
    if (he->twin->face != NULL)
      he->twin->face->hedge = he->twin->prev;
  }

  void HMesh::linkOverLinear(HalfEdge *he)
  {
    //the edges before and after [he] are
    //linked making the loop jump over [he]
    he->prev->next = he->next;
    he->next->prev = he->prev;
    
    //first hedge of vert and face are reset
    //in case they where set to [he]
    he->prev->vert->hedge = he->next;
    if (he->face != NULL)
      he->face->hedge = he->next;
  }
  
  void HMesh::transferIncomingEdges(HalfEdge *first, HalfEdge *last,
                                    Vertex *target) {
    
    //make all incoming edges of [first]'s vert
    //point to target vert until last edge is met
    
    HalfEdge *he = first;
    while (true) {
      he->vert = target;
      if (he == last) return;
      if (he->next == last) return;
      he = he->next->twin;
    }
  }
  
  void HMesh::transferIncomingEdges(Vertex *from, Vertex *to) {
    
    //make all incoming edges of [from] point to [to]
    
    HalfEdge *he = from->hedge->twin; do {
      he->vert = to;
      he = he->next->twin;
    } while (he != from->hedge->twin);
  }

  Vertex* HMesh::addVertex()
  {
    Vertex *vert = (Vertex*)classVertex->instantiate();
    vert->hedge = NULL;
    insertVert(vert);
    return vert;
  }

  /*
  -------------------------------------------------------------
  If a vertex is non-singular and lies on the mesh border,
  then it must have exactly 1 outgoing and 1 incoming halfedge
  with no face. If we found the two, its valid to link them
  to each other, to form the external mesh loop.

  When a vertex is singular there can be an infinite number
  of outgoing and incoming halfedges with no face, each of
  the pairs separating a group of faces where each of the
  faces in a group share an edge with one another.

  In a singular case it doesn't matter how the border loop
  of the halfedges flows, the only requirement being, that
  an incoming and an outgoing border edge belonging to the
  same group never get linked (this would 'detach' the
  faces from the rest of the mesh).
  ------------------------------------------------------------*/

  typedef LinkedList<HalfEdge*>::Iterator ManifoldIter;
  void HMesh::connectManifolds(LinkedList<HalfEdge*> *outManifolds)
  {  
    //Need manifolds!
    if (outManifolds->empty()) return;
    
    //Take first outgoing border edge
    HalfEdge *firstOut = outManifolds->first();
    outManifolds->popFront();

    //Walk incoming edges
    //(internal loops should be valid by now)
    HalfEdge *edge = firstOut->twin;    
    while (true) {
      
      //Only stop when the end of manifold reached
      if (edge->face == NULL) {
        
        //Check if we ran out of outgoing edges
        if (outManifolds->empty()){
          
          //Link with first one
          edge->next = firstOut;
          firstOut->prev = edge;
          return;
          
        }else{
          
          //Link with another manifold start
          //(this makes next incoming edge jump into that manifold)
          edge->next = outManifolds->first();
          outManifolds->first()->prev = edge;
          outManifolds->popFront();
        }
      }
      
      //Next incoming edge
      edge = edge->next->twin;
    }
  }

  /*
  -----------------------------------------------------------
  Adding a face is the only direct function allowing  mesh
  construction so this is where we need to check  for
  possible user errors. These can include:

  1) Operation would make one of the vertices singular
     - it doesn't lie on mesh border prior to face addition

  2) Operation would make the edge shared by 3 faces
     - the edge doesn't lie on the mesh border
  
  3) Wrong (opposite) indices orientation in relation to
     an existing edge
    
  4) Operation would make one of the vertices singular
     - a corner with both edges shared which completes a
     manifold (full face circle), but with another manifold
     inbetween
  ----------------------------------------------------------*/

  Face* HMesh::addFace (Vertex **vertices, int count)
  {
    //Check for invalid input
    if (count < 3) return NULL;
    /*
    //Walk pairs of vertices
    for (int i1=0; i1<count; ++i1) {
      int i2 = (i1+1)%count;
      int i3 = (i2+1)%count;

      bool anyBorder = false;
      HMesh::VertHedgeIter h1;
      HMesh::VertHedgeBackIter h2;
      HMesh::HalfEdge *firstShared = NULL;

      //A lone vertex with no adjacent face is ok
      if (vertices[i1]->outHedge() == NULL) continue;

      //Search for outgoing edge from v1 to v2
      for (h1.begin(vertices[i1]); !h1.end(); ++h1) {
        if (h1->parentFace() == NULL) anyBorder = true;
        if (h1->dstVertex() == vertices[i2]) {
          h2.begin(vertices[i2], h1->twinHedge());
          firstShared = *h1; }
      }
      
      //If edge not shared the vertex must be on border
      //else this will result in a singular vertex
      if (firstShared == NULL) {
        if (anyBorder) continue;
        else return NULL;
      }

      //The shared edge must be on border else this edge
      //will belong to more than 2 faces after addition
      if (firstShared->parentFace() != NULL)
        return NULL;

      //Walk over manifold around v2 until other border reached
      for (; !h2.end(); ++h2)
        if (h2->parentFace() == NULL)
          break;

      //If the second edge is shared with the same manifold it
      //is about to be closed, so no other manifold inbetween
      if (h2->dstVertex() == vertices[i3])
        if (firstShared->nextHedge() != *h2)
          return NULL;
    }
    */

    //Walk pairs of vertices
    for (int i1=0; i1<count; ++i1) {
      int i2 = (i1+1)%count;
      int i3 = (i2+1)%count;

      bool anyBorder = false;
      HMesh::VertHedgeIter hOut;
      HMesh::VertHedgeBackIter hIn;
      HMesh::HalfEdge *hShared = NULL;

      //A lone vertex with no adjacent face is ok
      if (vertices[i2]->outHedge() == NULL) continue;
      
      //Walk all round the v2 half edges
      for (hOut.begin(vertices[i2]); !hOut.end(); ++hOut) {
        //Search for an existing outgoing edge from v2 to v3
        if (hOut->dstVertex() == vertices[i3]) {
          hShared = *hOut; }
        //Search for a border edge
        if (hOut->parentFace() == NULL)
          anyBorder = true;
      }
      
      //v2 must be on the border
      //-- resolves condition (1)
      if (!anyBorder)
        return NULL;
      
      //Everyhing fine if edge (v2,v3) doesn't exist yet
      if (hShared == NULL) continue;
      
      //The shared edge must be on the border
      //-- resolves conditions (2),(3)
      if (hShared->parentFace() != NULL)
        return NULL;
      
      //Walk backwards over the manifold around v2 until another
      //border reached
      for (hIn.begin (vertices[i2], hShared->twinHedge()); !hIn.end(); ++hIn)
        if (hIn->parentFace() == NULL)
          break;
      
      //If edge (v1,v2) is the only other border edge around v2 then
      //there must not be another patch inbetween (v1,v2) and (v2,v3)
      //-- resolves condition (4)
      if (hIn->srcVertex() == vertices[i1])
        if (hIn->nextHedge() != hShared)
          return NULL;
    }
    
    //Create new face of specified class
    Face *face = (Face*)classFace->instantiate();
        
    //these lists hold outgoing adjacent edges with no face
    //for each vertex after generation of new edges
    LinkedList<HalfEdge*> *outManifolds =
      new LinkedList<HalfEdge*>[count];
    
    //this array holds internal edges of
    //the face to be linked into a loop
    HalfEdge **internalLoop = new HalfEdge* [count];
    
    
    //traverse consecutive pairs of vertices
    for (int i1=0; i1<count; ++i1) {
      int i2 = (i1+1)%count;
      
      //find an existing outgoing edge to our next vertex
      //->this would be a shared edge with existing face
      HalfEdge *shareFound = NULL;
      for (VertHedgeIter it(vertices[i1]); !it.end(); ++it) {
        //does it point to other edge end?
        if (it->dstVertex() == vertices[i2])
          {shareFound = *it; continue;}
        //is it an internal or border edge?
        if (it->parentFace() != NULL) continue;
        //is it outgoing edge?
        if (it->dstVertex() != vertices[i1])
          outManifolds[i1].pushBack(*it);
      }
      
      //is this edge shared?
      if (shareFound != NULL) {
        
        //assign face
        shareFound->face = face;
        //add to internal lopp
        internalLoop[i1] = shareFound;
        
      }else{
        
        //create two half-edges
        HalfEdge *eIn = (HalfEdge*)classHalfEdge->instantiate();
        HalfEdge *eOut = (HalfEdge*)classHalfEdge->instantiate();
        eIn->vert = vertices[i1];
        eOut->vert = vertices[i2];
        eIn->twin = eOut;
        eIn->face = NULL;
        eOut->twin = eIn;
        eOut->face = face;
        insertHalfEdge(eIn);
        insertHalfEdge(eOut);
        //create full-edge
        Edge *edge = (Edge*)classEdge->instantiate();
        edge->hedge = eOut;
        eIn->edge = edge;
        eOut->edge = edge;
        insertEdge(edge);
        //add outgoing half-edge to internal loop
        internalLoop[i1] = eOut;
        //add incoming half-edge to other vert's manifolds
        outManifolds[i2].pushBack(eIn);
      }
    }
    
    //link internal loop
    for (int l1=0; l1<count; l1++) {
      int l2 = (l1+1)%count;
      //link to next internal edge
      internalLoop[l1]->next = internalLoop[l2];
      internalLoop[l2]->prev = internalLoop[l1];
      //assign first edge to vertex if none
      if (vertices[l1]->hedge == NULL)
        vertices[l1]->hedge = internalLoop[l1];
    }
    

    //connect manifolds at each vertex
    for (int m=0; m<count; m++) {
      
      //refilter by removing edges that got face
      //assigned to the just created one
      for (ManifoldIter out=outManifolds[m].begin();
          out != outManifolds[m].end(); ++out)
        if ((*out)->face != NULL)
          out = outManifolds[m].removeAt(out);
      
      //finaly connect
      connectManifolds(&outManifolds[m]);
    }
    
    //free manifold lists
    delete[] outManifolds;
    
    //assign first edge to face
    face->hedge = internalLoop[0];
    insertFace(face);
    return face;
  }

  bool HMesh::connectVertices (Vertex *vert1, Vertex *vert2)
  {
    //Must be different vertices
    if (vert1 == vert2)
      return false;

    //Find common face and outgoing half-edges
    Face *face = NULL;
    HalfEdge *hedge1in, *hedge1out;
    HalfEdge *hedge2in, *hedge2out;

    for (HMesh::VertFaceIter vf1(vert1); !vf1.end(); ++vf1) {
      for (HMesh::VertFaceIter vf2(vert2); !vf2.end(); ++vf2) {

        if (*vf1 == *vf2) {
          face = *vf1;
          hedge1in = vf1.hedgeToVertex();
          hedge2in = vf2.hedgeToVertex();
          hedge1out = hedge1in->next;
          hedge2out = hedge2in->next;
          break;
        }}
      
      if (face != NULL)
        break;
    }

    //Must belong to the same face
    if (face == NULL)
      return false;
    
    //Must not be directly connected
    if (hedge1out->vert == vert2 || hedge2out->vert == vert1)
      return false;

    //Create new entities
    HalfEdge *newHedge1 = (HalfEdge*) classHalfEdge->instantiate();
    HalfEdge *newHedge2 = (HalfEdge*) classHalfEdge->instantiate();
    Edge *newEdge = (Edge*) classEdge->instantiate();
    Face *newFace = (Face*) classFace->instantiate();

    //Setup new entities
    newHedge1->vert = vert1;
    newHedge2->vert = vert2;
    newHedge1->face = face;
    newHedge2->face = newFace;
    newHedge1->edge = newEdge;
    newHedge2->edge = newEdge;
    newHedge1->next = hedge1out;
    newHedge2->next = hedge2out;
    newHedge1->prev = hedge2in;
    newHedge2->prev = hedge1in;
    newHedge1->twin = newHedge2;
    newHedge2->twin = newHedge1;
    newEdge->hedge = newHedge1;
    newFace->hedge = newHedge2;
    
    //Setup old entities
    hedge1in->next = newHedge2;
    hedge2in->next = newHedge1;
    hedge1out->prev = newHedge1;
    hedge2out->prev = newHedge2;

    //This might be pointing to wrong part of the loop
    face->hedge = newHedge1;

    //Adjust face pointers to both loops
    for (HalfEdge *h1 = hedge1out; h1 != newHedge1; h1=h1->next)
      h1->face = face;
    for (HalfEdge *h2 = hedge2out; h2 != newHedge2; h2=h2->next)
      h2->face = newFace;

    //Insert new entities
    insertHalfEdge( newHedge1 );
    insertHalfEdge( newHedge2 );
    insertEdge( newEdge );
    insertFace( newFace );

    return true;
  }

  void HMesh::removeFace (Face *face)
  {
    //traverse all edges/vertices
    bool deleteFirst = false;
    HalfEdge *he = face->hedge; do {
      
      HalfEdge *next = he->next;
      Vertex *vert = he->vert;
      
      //is this vert not shared?
      if (he->twin->face == NULL &&
          he->next->twin->face == NULL &&
          he->twin->prev == he->next->twin) {
        
        deleteVert(vert);
        
        //delete edge if not first
        if (he != face->hedge)
          deleteEdgeWhole(he->edge);
        else deleteFirst = true;
        
      }else{

        //current link at this vertex
        HalfEdge *link1 = he;
        HalfEdge *link2 = he->next;
        
        //is edge not shared?
        if (he->twin->face == NULL) {
          
          //take edge of another face
          link1 = he->twin->prev;
          //assign another edge to vert
          vert->hedge = link1->twin;
          
          //delete edge if not first
          if (he != face->hedge)
            deleteEdgeWhole(he->edge);
          else deleteFirst = true;
          
        }else{
          
          //mark external
          he->face = NULL;
        }
        
        //is next edge not shared?
        if (link2->twin->face == NULL) {
          
          //take edge of another face
          link2 = link2->twin->next;
          //assign another edge to vert
          vert->hedge = link2;
        }
        
        //link over non-shared edges at this vert
        if (link1 != he || link2 != he->next) {
          link1->next = link2;
          link2->prev = link1;
        }
      }
      
      he = next;
    } while (he != face->hedge);
    
    
    //delete first edge and face
    if (deleteFirst)
      deleteEdgeWhole(face->hedge->edge);

    deleteFace(face);
  }
  

  bool HMesh::removeEdge (Edge *edge)
  {
    //pick the two half-edges involved
    HalfEdge *hedge1 = edge->hedge;
    HalfEdge *hedge2 = edge->hedge->twin;
    
    //is this edge not shared?
    if (hedge1->face == NULL)
      return false;
    if (hedge2->face == NULL)
      return false;
    
    //pick the two faces involved
    Face *face1 = hedge1->face;
    Face *face2 = hedge2->face;
    
    //find the last common edge for face1
    while (hedge1->next->twin->face == face2) {
      //delete middle common edge and vert
      HalfEdge *next = hedge1->next;
      deleteVert(hedge1->vert);
      if (hedge1->twin != hedge2)
        deleteEdgeWhole(hedge1->edge);
      hedge1 = next;
    }
    
    //find the last common edge for face2
    while (hedge2->next->twin->face == face1) {
      //delete middle common edge and vert
      HalfEdge *next = hedge2->next;
      deleteVert(hedge2->vert);
      if (hedge2->twin != hedge1)
        deleteEdgeWhole(hedge2->edge);
      hedge2 = next;
    }
    
    //assign face2 to all remaining edges of face1
    HalfEdge *he = hedge1->next; do {
      he->face = face2;
      he = he->next;
    } while (he != hedge2->twin);
    
    //relink internal loop
    linkOverPerpendicular(hedge1);
    linkOverPerpendicular(hedge2);
    
    //delete last common edges
    deleteEdgeWhole(hedge1->edge);
    if (hedge2->edge != hedge1->edge)
      deleteEdgeWhole(hedge2->edge);
    
    //delete face
    deleteFace(face1);
    
    return true;
  }

  void HMesh::mergeEdges (HalfEdge *he1, HalfEdge *he2)
  {
    //Two edges of a triangle are merged into one
    //preparing the ground for third edge collapse
    
    //Edge of [he1] gets removed, Edge of [he2] stays
    //[he1] should be outgoing to third edge
    //[he2] should be incoming to third edge
    /*
    if (he1 == he2)
      printf("MERGE ERROR! edges the same\n");
    if (he1->twin == he2)
      printf("MERGE ERROR! edge2 twin to edge1\n");
    if (he2->twin == he1)
      printf("MERGE ERROR! edge1 twin to edge2\n");
    if (he1->twin == he2->twin)
      printf("MERGE ERROR! edges have the same twin\n");
    if (he1->twin == he1)
      printf("MERGE ERROR! edge1 twin to itself!\n");
    if (he2->twin == he2)
      printf("MERGE ERROR! edge2 twin to itself!\n");*/
    he1->twin->twin = he2->twin;
    he2->twin->twin = he1->twin;
    he1->twin->edge = he2->edge;
    /*if (he1->twin->twin == he2->twin->twin)
      printf("POST MERGE ERROR! edges have the same twin\n");*/
    
    //first hedge of remaining verts and edge are
    //reset in case they where set to [he1] or [he2]
    he2->vert->hedge = he2->twin;
    he2->twin->vert->hedge = he2->twin->next;
    he2->twin->edge->hedge = he2->twin;
    
    //two half edges and an edge get deleted
    deleteEdge(he1->edge);
    deleteHalfEdge(he1);
    deleteHalfEdge(he2);
  }


  bool HMesh::collapseEdge (Edge *edge)
  {
    //pick the hedges and verts of edge
    HalfEdge *hedge1 = edge->hedge;
    HalfEdge *hedge2 = edge->hedge->twin;
    Vertex *vert1 = hedge1->vert;
    Vertex *vert2 = hedge2->vert;
    int vert1deg = vert1->degree();
    int vert2deg = vert2->degree();
    
    
    //check left and right loop if they are triangles
    HalfEdge *check[2] = {hedge1, hedge2};
    HalfEdge *tris[2] = {NULL, NULL};
    int trisCount = 0;
    
    for (int c=0, c2=1; c<2; c++, c2--) {

      //is it a triangle?
      if (check[c]->next->vert ==
          check[c]->prev->twin->vert) {

        //does it have a face?
        if (check[c]->face != NULL)
          tris[trisCount++] = check[c];
        else
          //is it different than the other loop?
          if (check[c]->next->twin != check[c2]->prev &&
              check[c]->prev->twin != check[c2]->next)
            tris[trisCount++] = check[c];
      }
    }
    
    //check for degenerate triangles on sides
    for (int t=0; t<trisCount; t++) {
      HalfEdge *mergeOut = tris[t]->next;
      HalfEdge *mergeIn = tris[t]->prev;
      
      if (mergeOut->vert->degree() == 3 &&
          mergeOut->twin->face != NULL &&
          mergeIn->twin->face != NULL) {

        if (mergeOut->twin->isLoopTriangle())
            return false;
        if (mergeIn->twin->isLoopTriangle())
            return false;
      }}
    
    //check for degenerate triangle on top/bottom
    if (trisCount == 2) {
      
      if (vert1deg == 3 && vert2deg == 3 &&
          hedge1->next->twin->face != NULL &&
          hedge2->next->twin->face != NULL) {

        if (hedge1->next->twin->isLoopTriangle())
          return false;
        if (hedge2->next->twin->isLoopTriangle())
          return false;
      }}
    
    
    //transfer edges
    transferIncomingEdges(vert1, vert2);
    
    
    //merge triangles loops
    for (int t=0; t<trisCount; t++) {
      
      HalfEdge *mergeOut = tris[t]->next;
      HalfEdge *mergeIn = tris[t]->prev;
      HalfEdge *mergeOutTwin = mergeOut->twin;
      HalfEdge *mergeInTwin = mergeIn->twin;
      Face *face = mergeOut->face;
      
      //merge down triangle
      mergeEdges(mergeOut, mergeIn);
      if (face != NULL) deleteFace(face);
        
      //if this is a triangle on boundary it
      //should have a face and side edges not shared
      if (face != NULL &&
          mergeOutTwin->face == NULL &&
          mergeInTwin->face == NULL) {
        
        //link over at collapsed vert
        linkOverPerpendicular(mergeOutTwin);
        
        //is third vert shared (manifolds touch)?
        if (mergeOut->vert->degree() > 2)
          //link over at third vert
          linkOverPerpendicular(mergeInTwin);
        else
          deleteVert(mergeOut->vert);
        
        //whole merged edge can be removed
        deleteEdgeWhole(mergeOutTwin->edge);
      }
    }
    
    //link over collapsed edge
    //if loops not merged
    if (hedge1->next->valid)
      linkOverLinear(hedge1);
    if (hedge2->next->valid)
      linkOverLinear(hedge2);
    
    //remove collapsed edge and vert
    deleteEdgeWhole(hedge1->edge);
    deleteVert(vert1);
    
    //remove both verts if both triangles
    //removed and both vertices unshared
    if (!hedge1->prev->edge->valid &&
        !hedge2->prev->edge->valid &&
        vert1deg<=3 && vert2deg<=3)
        deleteVert(vert2);
    
    return true;
  }

  bool HMesh::weldVertices(Vertex *vert1, Vertex *vert2)
  { 
    //collapse edge if verts connected
    HalfEdge *h = vert1->outHedgeTo(vert2);
    if (h != NULL) return collapseEdge(h->edge);
    
    //find 2 boundary hedges of vert1
    HalfEdge *bound1out = NULL;
    HalfEdge *bound1in = NULL;
    for (VertHedgeIter e1(vert1); !e1.end(); ++e1) {
      //save bound edge or return if more
      if ((*e1)->face == NULL) {
        if (bound1out == NULL)
          bound1out = (*e1);
        else if (bound1in == NULL)
          bound1in = (*e1);
        else return false;
      }}
    
    //find 2 boundary hedges of vert2
    HalfEdge *bound2out = NULL;
    HalfEdge *bound2in = NULL;
    for (VertHedgeIter e2(vert2); !e2.end(); ++e2) {
      //save bound edge or return if more
      if ((*e2)->face == NULL) {
        if (bound2out == NULL)
          bound2out = (*e2);
        else if (bound2in == NULL)
          bound2in = (*e2);
        else return false;
      }}
    
    
    //swap in and out properly
    if (bound1out->vert == vert1) {
      HalfEdge *bound1temp = bound1out;
      bound1out = bound1in;
      bound1in = bound1temp;}
    if (bound2out->vert == vert2) {
      HalfEdge *bound2temp = bound2out;
      bound2out = bound2in;
      bound2in = bound2temp;}
    

    //check if left or right triangle to merge
    HalfEdge *mergeOut[2] = {NULL, NULL};
    HalfEdge *mergeIn[2] = {NULL, NULL};
    int mergeCount=0;
    
    if (bound1out->next == bound2in) {
      mergeOut[mergeCount] = bound1out;
      mergeIn[mergeCount++] = bound2in;}
    if (bound2out->next == bound1in) {
      mergeOut[mergeCount] = bound2out;
      mergeIn[mergeCount++] = bound1in;
    }
    
    //check degenerate triangles on side
    for (int m=0; m<mergeCount; ++m) {
      if (mergeOut[m]->vert->degree() == 3) {
        if (mergeOut[m]->twin->isLoopTriangle())
          return false;
        if (mergeIn[m]->twin->isLoopTriangle())
          return false;
      }}
    
    //check degenerate triangles on top/bottom
    if (mergeCount == 2) {
      if (vert1->degree()==2 && vert2->degree()==2){
        if (bound1out->twin->isLoopTriangle())
          return false;
        if (bound2out->twin->isLoopTriangle())
          return false;
      }}
    
    
    //transfer edges
    transferIncomingEdges(vert1, vert2);
    
    //merge triangles
    for (int m=0; m<mergeCount; ++m)
      mergeEdges(mergeOut[m],mergeIn[m]);
    
    
    //link over welded vertex if
    //loops not merged
    if (bound1out->valid) {
      bound2in->next = bound1out;
      bound1out->prev = bound2in;}
    if (bound2out->valid) {
      bound1in->next = bound2out;
      bound2out->prev = bound1in;}
    
    //delete vert1
    deleteVert(vert1);
    
    return true;
  }

  HalfEdge* HMesh::findBoundary()
  {
    for (HedgeIter e(this); !e.end(); ++e)
      if ((*e)->face == NULL)
        return *e;
    
    return NULL;
  }

}//namespace GE
