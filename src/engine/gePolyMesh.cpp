#include "gePolyMesh.h"
#include "engine/geMatrix.h"

namespace GE
{
  DEFINE_CLASS (PolyMesh);
  DEFINE_CLASS (PolyMesh::Vertex);
  DEFINE_CLASS (PolyMesh::HalfEdge);
  DEFINE_CLASS (PolyMesh::Edge);
  DEFINE_CLASS (PolyMesh::Face);

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
    ((PolyMesh::HalfEdge*)he)->vnormal = &dummyVertexNormal;
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
    Vector3 &p1 = f->firstHedge()->dstVertex()->point;
    Vector3 &p2 = f->firstHedge()->nextHedge()->dstVertex()->point;
    Vector3 &p3 = f->firstHedge()->nextHedge()->nextHedge()->dstVertex()->point;
    Vector3 s1 = p2 - p1;
    Vector3 s2 = p3 - p1;
    f->normal = Vector::Cross( s2, s1 ).normalize();
    f->center = (p1 + p2 + p3) / 3;
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

    SmoothEdgeGroup() : faceCount(0), vnormal(NULL) {}

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
      updateFaceNormal(*f);
    
    switch (metric)
    {
    case SmoothMetric::None:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalFlat (*v);
      break;

    case SmoothMetric::All:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalSmooth(*v);
      break;

    case SmoothMetric::Face:
      
      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalGroups(*v);
      break;
      
    case SmoothMetric::Edge:

      for (PolyMesh::VertIter v(this); !v.end(); ++v)
        updateVertNormalEdges(*v);
      break;
    }
  }

  /*
  ----------------------------------------------------
  Triangulation
  ----------------------------------------------------*/

  struct TrigNode
  {
    Vertex *vertex;
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

  void PolyMesh::triangulate (bool smoothEdges)
  {
    int vertcount = 0;
    int polycount = 0;

    ArrayList< TrigEdge > trigEdges;

    for (PolyMesh::VertIter v(this); !v.end(); ++v)
    {
      vertcount++;
      v->tag.id = vertcount;
    }

    for (PolyMesh::FaceIter f(this); !f.end(); ++f)
    {
      polycount++;
      if (polycount == 10163)
        int oooo = 1;

      PolyMesh::FaceVertIter fv;
      UintSize numavg[3] = {0,0,0};
      UintSize avgi = 0;
      Vector3 avg[3];

      //Split vertices into three groups
      for (fv.begin(*f); !fv.end(); ++fv, ++avgi)
        numavg[ avgi % 3 ] += 1;

      //Skip if triangle
      if (avgi == 3)
        continue;

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

      /*
      //Find the most convex corner
      Float trigNorm = 0.0f;
      Vector3 s1, s2, trigN, trigX, trigY;
      PolyMesh::FaceVertIter fvPrev( *f );
      PolyMesh::FaceVertIter fvCur( *f );
      PolyMesh::FaceVertIter fvNext( *f );
      ++fvCur; ++fvNext; ++fvNext;
      bool firstN = true;

      for ( ; !fvPrev.end(); ++fvPrev, ++fvCur, ++fvNext)
      {
        Vector3 s1 = fvPrev->point - fvCur->point;
        Vector3 s2 = fvNext->point - fvCur->point;
        Vector3 N = Vector::Cross( s2, s1 );
        Float norm = N.norm();
        
        if (firstN || norm > trigNorm)
        {
          trigNorm = norm;
          trigN = N;
          trigX = s1;
          firstN = false;
        }
      }

      //Calculate triangulation space if good enough
      if (trigNorm < -0.001f || trigNorm > +0.001f)
      {
        trigN *= (1.0f / trigNorm);
        trigX = trigX.normalize();
        trigY = Vector::Cross( trigN, trigX );
      }
      else continue;*/

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

      //Cut ears until triangle or no ears found
      prev.begin( trigNodes ); --prev;
      cur.begin( trigNodes );
      next.begin( trigNodes ); ++next;

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
            TrigEdge e;
            e.face = *f;
            e.vertex1 = prev->vertex;
            e.vertex2 = next->vertex;
            trigEdges.pushBack( e );

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
    }

    //Create new edges
    for (UintSize e=0; e<trigEdges.size(); e++)
    {
      //Connect the vertices
      TrigEdge &edge = trigEdges[e];
      if (!connectVertices( edge.vertex1, edge.vertex2 )) continue;

      //Find two adjacent faces
      HalfEdge *hedge1 = edge.vertex1->outHedgeTo( edge.vertex2 );
      HalfEdge *hedge2 = edge.vertex2->outHedgeTo( edge.vertex1 );
      Face *face1 = hedge1->parentFace();
      Face *face2 = hedge2->parentFace();

      //Copy groups and material IDs off the original face
      face1->smoothGroups = face2->smoothGroups = edge.face->smoothGroups;
      setMaterialID( face1, edge.face->materialID() );
      setMaterialID( face2, edge.face->materialID() );

      //Make the edge smooth if requested
      hedge1->fullEdge()->isSmooth = smoothEdges;
    }
  }

}//namespace GE
