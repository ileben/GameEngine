#include "gePolyMesh.h"
#include "math/geMatrix.h"

namespace GE
{
  DEFINE_CLASS (PolyMesh);
  DEFINE_CLASS (PolyMesh::Vertex);
  DEFINE_CLASS (PolyMesh::HalfEdge);
  DEFINE_CLASS (PolyMesh::Edge);
  DEFINE_CLASS (PolyMesh::Face);
  DEFINE_CLASS (PolyMesh::Triangle);

  typedef PolyMesh::Vertex Vertex;
  typedef PolyMesh::HalfEdge HalfEdge;
  typedef PolyMesh::Edge Edge;
  typedef PolyMesh::Face Face;
  typedef PolyMesh::VertexNormal VertexNormal;

  PolyMesh::PolyMesh ()
  {
    for (MaterialID m=0; m<GE_MAX_MATERIAL_ID; ++m)
      facesPerMaterial [m] = 0;
  }
  
  /*
  --------------------------------------------------
  Make sure a freshly inserted half edge has got
  a valid smooth normal even before updateNormals()
  is called.
  --------------------------------------------------*/

  void PolyMesh::insertHalfEdge (HMesh::HalfEdge *he)
  {
    HMesh::insertHalfEdge(he);
    PolyMesh::HalfEdge *phe = (PolyMesh::HalfEdge*)he;
    phe->vnormal = &dummyVertexNormal;
    phe->vtangent = &dummyVertexTangent;
  }
  
  /*
  ----------------------------------------------------
  Material ID handling
  ----------------------------------------------------*/
  
  void PolyMesh::addMaterialID (MaterialID m)
  {
    facesPerMaterial[ m ]++;
    if( facesPerMaterial[ m ] == 1 )
      if(! materialsUsed.contains( m ))
        materialsUsed.pushFront( m );
  }
  
  void PolyMesh::subMaterialID (MaterialID m)
  {
    facesPerMaterial[ m ]--;
    if( facesPerMaterial[ m ] == 0 )
      materialsUsed.remove( m );
  }
  
  void PolyMesh::insertFace (HMesh::Face *f)
  {
    HMesh::insertFace(f);
    addMaterialID( ((PolyMesh::Face*) f)->materialID() );
  }
  
  ListHandle PolyMesh::deleteFace (HMesh::Face *f)
  {
    subMaterialID( ((PolyMesh::Face*) f)->materialID() );
    return HMesh::deleteFace(f);
  }
  
  void PolyMesh::setMaterialID (Face *f, MaterialID id)
  {
    if( id == f->materialID() ) return;
    subMaterialID( f->materialID() );
    addMaterialID( id );
    f->matId = id;
  }

  /*
  --------------------------------------------------
  Calculates face normal from first three vertices
  --------------------------------------------------*/

  void PolyMesh::updateFaceNormal (Face *f)
  {
    int tricount = 0;
    f->normal.set( 0,0,0 );

    for (FaceTriIter ft(f); !ft.end(); ++ft)
    {
      Vector3 &p1 = ft->hedges[0]->dstVertex()->point;
      Vector3 &p2 = ft->hedges[1]->dstVertex()->point;
      Vector3 &p3 = ft->hedges[2]->dstVertex()->point;

      Vector3 s1 = p2 - p1;
      Vector3 s2 = p3 - p1;

      f->normal += Vector::Cross( s2, s1 ).normalize();
      tricount += 1;
    }

    if (tricount > 0)
      f->normal /= (Float) tricount;
  }
  
  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex.
  Normals are not averaged over the adjacent
  faces which results in a different normal
  being used for each adjacent face.
  -------------------------------------------------*/
  
  void PolyMesh::updateVertNormalFlat (Vertex *vert)
  {
    VertFaceIter f;
    
    //Take the normal off each incident face and store it
    for (f.begin(vert); !f.end(); ++f) {
      vertexNormals.pushBack (VertexNormal (f->normal));
      vertexNormals.last().vert = vert;
      f.hedgeToVertex()->vnormal = &vertexNormals.last();
    }
  }
  
  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex.
  Smoothing groups are not taken into account
  which results in a single normal being used for
  each adjacent face.
  -------------------------------------------------*/
  
  void PolyMesh::updateVertNormalSmooth (Vertex *vert)
  {
    int count = 0;
    Vector3 sum;
    VertFaceIter f;

    //Pass1: accumulate adjacent face normals
    for (f.begin(vert); !f.end(); ++f) {
      sum += f->normal;
      count++; }

    //Average the final normal vector
    if (count > 0) sum /= (Float)count;
    vertexNormals.pushBack (VertexNormal (sum));
    vertexNormals.last().vert = vert;

    //Pass2: store normal into each half-edge
    for (f.begin(vert); !f.end(); ++f) {
      f.hedgeToVertex()->vnormal = &vertexNormals.last();
    }
  }


  /*
  -------------------------------------------------
  This class provides basic operations for adding
  faces to and merging smoothing groups to make
  the normal calculation code more readable.
  -------------------------------------------------*/

  class SmoothGroup
  {
  public:
    Uint32 mask;
    Vector3 normal;
    int faceCount;
    Face *zeroFace;
    VertexNormal *vnormal;

    SmoothGroup () {
      mask = 0;
      faceCount = 0;
      zeroFace = NULL;
      vnormal = NULL;
    }

    SmoothGroup (Face *f) {
      mask = f->smoothGroups;
      normal = f->normal;
      faceCount = 1;
      zeroFace = (mask==0) ? f : NULL;
      vnormal = NULL;
    }

    SmoothGroup& operator+= (SmoothGroup &g) {
      mask |= g.mask;
      normal += g.normal;
      faceCount += g.faceCount;
      return *this;
    }

    SmoothGroup& operator+= (Face *f) {
      mask |= f->smoothGroups;
      normal += f->normal;
      faceCount++;
      return *this;
    }

    /*
    No two faces with 0 smoothing flags are considered to be in
    the same group. However, zeroFace is stored so that a face with
    0 flag matches it's own SmoothGroup if tested again after creation */
    bool operator== (Face *f) {
      return ((mask & f->smoothGroups) != 0) || (zeroFace == f);
    }
  };


  /*
  -------------------------------------------------
  Calculates per-face normals for given vertex
  according to smoothing groups and stores them
  into half-edges pointing to that vertex in each
  of the adjacent faces.
  -------------------------------------------------*/

  void PolyMesh::updateVertNormalGroups(Vertex *vert)
  {
    PolyMesh::VertFaceIter f;
    ArraySet<SmoothGroup> groups(8);
    int g = 0;

    //Walk the adjacent faces and merge smooth groups
    for (f.begin(vert); !f.end(); ++f)
    {
      //Walk the existing groups
      int matchCount = 0;
      SmoothGroup *firstMatch = NULL;
      for (g=0; g<groups.size(); )
      {
        //Add normal if any common smoothing group
        if (groups[g] == *f) {
          groups[g] += (*f);

          //Store first match
          if (++matchCount == 1) {
            firstMatch = &groups[g];
          }else {

            //Merge multiple matches
            (*firstMatch) += groups[g];
            groups.removeAt (g);
            continue;
          }
        }

        g++; } //Walk groups

      //Create a new group
      if (matchCount == 0)
        groups.add (SmoothGroup (*f));

    }//Walk faces


    //Average out the group normals and create
    //a new vertex normal for each
    for (g=0; g<groups.size(); ++g)
    {
      groups[g].normal /= (Float)groups[g].faceCount;
      vertexNormals.pushBack (VertexNormal (groups[g].normal));
      vertexNormals.last().vert = vert;
      groups[g].vnormal = &vertexNormals.last();
    }
    
    //Pass2: Apply unique normals to faces
    for (f.begin(vert); !f.end(); ++f)
    {
      //Find the matching merged group
      for (g=0; g<groups.size(); ++g) {
        if (groups[g] == *f) {
          f.hedgeToVertex()->vnormal = groups[g].vnormal;
          break;
        }}
    }
  }

  class SmoothEdgeGroup
  {
  public:
    Vector3 normal;
    int faceCount;
    VertexNormal *vnormal;

    SmoothEdgeGroup() {
      faceCount = 0;
      vnormal = NULL;
    }

    SmoothEdgeGroup& operator+= (Face* f) {
      normal += f->normal;
      faceCount += 1;
      return *this;
    }
  };

  void PolyMesh::updateVertNormalEdges (Vertex *vert)
  {
    UintSize g;
    SmoothEdgeGroup *curGroup = NULL;
    PolyMesh::VertFaceIter vf;
    ArrayList<SmoothEdgeGroup> groups;

    //Pass1: find first hard edge
    for (vf.begin( vert ); !vf.end(); ++vf)
      if ( ! vf.hedgeToVertex()->fullEdge()->isSmooth)
        break;

    //Pass2: sum normals into groups
    for (vf.begin( vf ); !vf.end(); ++vf)
    {
      //Check if the edge hard (or its the first group and no hard edge)
      if ( ! vf.hedgeToVertex()->fullEdge()->isSmooth || curGroup==NULL) {

        //Create a new smooth group
        groups.pushBack( SmoothEdgeGroup() );
        curGroup = &groups.last();
      }

      //Add face to the current group
      (*curGroup) += *vf;
    }

    //Average the group normals and create
    //a new vertex normal for each
    for (g=0; g<groups.size(); ++g)
    {
      groups[g].normal /= (Float)groups[g].faceCount;
      vertexNormals.pushBack (VertexNormal (groups[g].normal));
      vertexNormals.last().vert = vert;
      groups[g].vnormal = &vertexNormals.last();
    }

    //Assign normals to faces
    for (vf.begin( vf ), g=0; g < groups.size(); ++g)
      for (int f=0; f < groups[g].faceCount; ++f, ++vf)
        vf.hedgeToVertex()->vnormal = groups[g].vnormal;
  }

  /*
  ----------------------------------------------------
  Updates face and vertex normals for the whole mesh
  ----------------------------------------------------*/

  void PolyMesh::updateNormals (SmoothMetric::Enum metric)
  {
    vertexNormals.clear();
    
    for (PolyMesh::FaceIter f(this); !f.end(); ++f)
      updateFaceNormal( *f );
    
    switch (metric)
    {
    case SmoothMetric::None:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalFlat( *v );
      break;

    case SmoothMetric::All:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalSmooth( *v );
      break;

    case SmoothMetric::Face:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalGroups( *v );
      break;
      
    case SmoothMetric::Edge:

      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalEdges( *v );
      break;
    }
  }

  /*
  -------------------------------------------------------
  Tangents for normal mapping
  -------------------------------------------------------*/

  /*
  Calculating tangent space:
  We have a triangle whose vertex positions are P1, P2, and P3,
  and the texture coordinates are U1, U2, and U3.
  Let:

    Q1 = P2 - P1
    Q2 = P3 - P1
    R1 = U2 - U1
    R2 = U3 - U1

  If T (tangent) and B (bitangent) represent unit vectors of the
  texture coordinate space within the object's 3D space then the
  following two equations hold:

    Q1 = R1x * T + R1y * B
    Q2 = R2x * T + R2y * B

  By writing this in matrix form we get

    |Q1x Q1y Q1z| = |R1x R1y| * |Tx Ty Tz|
    |Q2x Q2y Q2z|   |R2x R2y|   |Bx By Bz|

  Multiplying both sides by the inverse of the R matrix, we get

    |Tx Ty Tz| =       1 /        * | R2y -R1y| * |Q1x Q1y Q1z|
    |Bx By Bz|   R1xR2y - R2xR1y    |-R2x  R1x|   |Q2x Q2y Q2z|

  */


  void PolyMesh::updateFaceTangent (Face *f, TexMesh::Face *tf)
  {
    int tricount = 0;
    f->tangent.set( 0,0,0 );
    f->bitangent.set( 0,0,0 );
    
    for (FaceTriIter ft(f); !ft.end(); ++ft)
    {
      PolyMesh::Vertex *P1 = ft->hedges[0]->dstVertex();
      PolyMesh::Vertex *P2 = ft->hedges[1]->dstVertex();
      PolyMesh::Vertex *P3 = ft->hedges[2]->dstVertex();

      TexMesh::Vertex *U1 = (TexMesh::Vertex*) P1->tag.ptr;
      TexMesh::Vertex *U2 = (TexMesh::Vertex*) P2->tag.ptr;
      TexMesh::Vertex *U3 = (TexMesh::Vertex*) P3->tag.ptr;

      Vector3 Q1 = P2->point - P1->point;
      Vector3 Q2 = P3->point - P1->point;
      Vector2 R1 = U2->point - U1->point;
      Vector2 R2 = U3->point - U1->point;

      Float D = 1.0f / (R1.x * R2.y - R2.x * R1.y);
      
      Vector3 T,B;
      T.x = ( R2.y * Q1.x - R1.y * Q2.x) * D;
      T.y = ( R2.y * Q1.y - R1.y * Q2.y) * D;
      T.z = ( R2.y * Q1.z - R1.y * Q2.z) * D;
      B.x = (-R2.x * Q1.x + R1.x * Q2.x) * D;
      B.x = (-R2.x * Q1.y + R1.x * Q2.y) * D;
      B.x = (-R2.x * Q1.z + R1.x * Q2.z) * D;

      f->tangent += T;
      f->tangent += B;
    }
    
    if (tricount > 0)
    {
      f->tangent /= (Float) tricount;
      f->bitangent /= (Float) tricount;
    }
  }

  void PolyMesh::updateVertTangent (Vertex *v, TexMesh::Vertex *tv)
  {

  }

  void PolyMesh::updateTangents (TexMesh *texMesh)
  {
    PolyMesh::VertIter v;
    PolyMesh::FaceIter f;
    PolyMesh::FaceVertIter fv;
    TexMesh::FaceIter tf;
    TexMesh::FaceVertIter tfv;

    //Store tex vertex pointers into poly vertex tags
    for (f.begin(this), tf.begin(texMesh); !f.end(); ++f, ++tf)
      for (fv.begin(*f), tfv.begin(*tf); !fv.end(); ++fv, ++tfv)
        fv->tag.ptr = *tfv;

    //Calculate tangents
    vertexTangents.clear();

    for (f.begin(this); !f.end() && !tf.end(); ++f, ++tf)
      updateFaceNormal( *f );

    for (v.begin(this); !v.end(); ++v)
      updateVertTangent( *v, (TexMesh::Vertex*) v->tag.ptr );
  }

  /*
  ----------------------------------------------------
  Triangulation
  ----------------------------------------------------*/

  struct TrigNode
  {
    Vertex *vertex;
    HalfEdge *hedge;
    Vector2 point;
    bool orientation;
  };

  struct TrigEdge
  {
    Face *face;
    Vertex *vertex1;
    Vertex *vertex2;
  };

  typedef LinkedList<TrigNode>::Iterator TrigIter;
  typedef LinkedList<TrigNode>::CyclicIterator TrigCyclIter;

  bool findNodeOrientation (const TrigNode &prev, TrigNode &cur, const TrigNode &next,
                            bool convex, bool convexInit)
  {
    Vector2 prevV = (prev.point - cur.point).normalize();
    Vector2 nextV = (next.point - cur.point).normalize();
    Float cross = Vector::Cross( prevV, nextV );

    if (!convexInit || cross < -0.01f || cross > 0.01f)
      cur.orientation = (cross > 0.0f);
    else cur.orientation = !convex;

    return cur.orientation;
  }
  
  bool findNodeOrientation (const TrigCyclIter &cur, bool convex, bool convexInit)
  {
    TrigCyclIter prev( cur ); --prev;
    TrigCyclIter next( cur ); ++next;
    return findNodeOrientation( *prev, *cur, *next, convex, convexInit );
  }

  void PolyMesh::triangulate ()
  {
    clearTriangles();
    //int polycount = 0;

    for (PolyMesh::FaceIter f(this); !f.end(); ++f)
    {
      //polycount++;
      //if (polycount == 10163)
        //int oooo = 1;

      PolyMesh::FaceVertIter fv;
      UintSize numavg[3] = {0,0,0};
      UintSize avgi = 0;
      Vector3 avg[3];

      //Split vertices into three groups
      for (fv.begin(*f); !fv.end(); ++fv, ++avgi)
        numavg[ avgi % 3 ] += 1;

      //Exit early if triangle
      if (avgi == 3)
      {
        addTriangle( *f,
          f->firstHedge()->prevHedge(),
          f->firstHedge(),
          f->firstHedge()->nextHedge());
        continue;
      }

      //Average vertices in each group
      fv.begin(*f);
      for (int g=0; g<3; ++g)
        for (avgi=0; avgi<numavg[g]; ++avgi, ++fv)
          avg[g] += fv->point;

      avg[0] /= (Float) numavg[0];
      avg[1] /= (Float) numavg[1];
      avg[2] /= (Float) numavg[2];

      //Define triangulation space
      Vector3 s1 = (avg[1] - avg[0]).normalize();
      Vector3 s2 = (avg[2] - avg[0]).normalize();
      Vector3 trigN = Vector::Cross( s2, s1 ).normalize();
      Vector3 trigY = Vector::Cross( trigN, s1 );
      Vector3 trigX = s1;

      //Construct world-to-trig matrix
      Matrix4x4 trigM;
      trigM.setColumn( 0, trigX );
      trigM.setColumn( 1, trigY );
      trigM.setColumn( 2, trigN );
      trigM.setColumn( 3, fv->point );
      trigM = trigM.affineInverse();
      
      //Transform polygon into trig space and find bottom-left point
      LinkedList< TrigNode > trigNodes;
      bool minFirst = true;
      TrigIter minI;

      for (fv.begin(*f); !fv.end(); ++fv) {
        
        TrigNode node;
        node.vertex = *fv;
        node.hedge = fv.hedgeToVertex();
        node.point = ( trigM * fv->point ).xy();
        TrigIter newI = trigNodes.pushBack( node );
        
        if (minFirst)
          minI = newI;
        else if (node.point.x < minI->point.x)
          minI = newI;
        else if (node.point.x == minI->point.x && node.point.y < minI->point.y)
          minI = newI;
      
        minFirst = false;
      }
      
      //Find orientation for all corners
      Vector2 prevV, nextV;
      TrigCyclIter prev, cur, next;
      bool convex = false, convexInit = false;

      prev.begin( trigNodes, minI ); --prev;
      cur.begin( trigNodes, minI );
      next.begin( trigNodes, minI ); ++next;

      for ( ; !cur.end(); ++cur, ++prev, ++next)
      {
        findNodeOrientation( *prev, *cur, *next, convex, convexInit );
        if (!convexInit) { convex = cur->orientation; convexInit = true; }
      }

      //Rest corner iterators
      prev.begin( trigNodes ); --prev;
      cur.begin( trigNodes );
      next.begin( trigNodes ); ++next;

      //Cut ears until triangle or no ears found
      while (trigNodes.size() > 3 && !cur.end())
      {
        //Find next convex node
        if (cur->orientation == convex)
        {
          //Check if a concave node is inside
          bool isEar = true;
          for (TrigCyclIter it( cur ); !it.end(); ++it) {
            if (it == prev || it == cur || it == next) continue;
            if (it->orientation == convex) continue;
            if (Vector::InsideTriangle( it->point, prev->point, cur->point, next->point))
              { isEar = false; break; }
          }

          //Cut an ear
          if (isEar)
          {
            addTriangle( *f, prev->hedge, cur->hedge, next->hedge );
            trigNodes.removeAt( cur );
            findNodeOrientation( prev, convex, convexInit );
            findNodeOrientation( next, convex, convexInit );
            cur.begin( next ); ++next;
            continue;
          }
        }

        //Next corner
        ++prev; ++cur; ++next;
      }

      //Cut the remaining corners
      while (next != prev)
      {
        addTriangle( *f, prev->hedge, cur->hedge, next->hedge );
        ++cur; ++next;
      }

    }//Walk faces
  }

  void PolyMesh::addTriangle (Face *f, HalfEdge *h1, HalfEdge *h2, HalfEdge *h3)
  {
    //Create new triangle
    Triangle tri;
    tri.hedges[0] = h1;
    tri.hedges[1] = h2;
    tri.hedges[2] = h3;
    tri.next = NULL;
    triangles.pushBack(tri);
    Triangle *newTri = &triangles.last();

    //Check if first for this face
    if (f->triangle == NULL) {
      f->triangle = newTri;
      return;
    }

    //Find last triangle on face
    Triangle *t = f->triangle;    
    while (t->next != NULL)
      t = t->next;

    //Append new one
    t->next = newTri;
  }

  void PolyMesh::clearTriangles()
  {
    for (PolyMesh::FaceIter f(this); !f.end(); ++f)
      f->triangle = NULL;

    triangles.clear();
  }

}//namespace GE
