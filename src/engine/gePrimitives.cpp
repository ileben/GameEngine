#include "gePrimitives.h"

namespace GE
{
  DEFINE_CLASS( CubeMesh );
  DEFINE_CLASS( SphereMesh );
  

  void CubeMesh::addQuad (const Vector3 &normal,
                          int i1, int i2, int i3, int i4,
                          Vector3 *points)
  {
    binding.init( getFormat() );

    //First index in the final data array
    VertexID ifirst = (VertexID) data.size();
    
    //Indices into the given array of points
    int *pi[4] = { &i1, &i2, &i3, &i4 };
    
    //Add vertices at 4 given points from the
    //given array and with given normal
    for (int i=0; i<4; ++i)
    {
      TriVertex vert = binding( addVertex() );
      *vert.coord = points[ *pi[i] ];
      *vert.normal = normal;
    }
    
    //Add 2 triangle faces
    addFace( ifirst+0, ifirst+1, ifirst+2 );
    addFace( ifirst+2, ifirst+3, ifirst+0 );
  }
  
  CubeMesh::CubeMesh ()
  {
    //Coordinates of the 8 corners
    Vector3 points[8] =
    {
      Vector3( -1, -1, -1 ),
      Vector3( +1, -1, -1 ),
      Vector3( +1, -1, +1 ),
      Vector3( -1, -1, +1 ),

      Vector3( -1, +1, -1 ),
      Vector3( +1, +1, -1 ),
      Vector3( +1, +1, +1 ),
      Vector3( -1, +1, +1 )
    };

    //Construct mesh
    addFaceGroup( 0 );
    addQuad( Vector3( 0,-1, 0 ), 3,2,1,0, points );
    addQuad( Vector3( 0, 1, 0 ), 4,5,6,7, points );
    addQuad( Vector3( 0, 0,-1 ), 5,4,0,1, points );
    addQuad( Vector3( 1, 0, 0 ), 6,5,1,2, points );
    addQuad( Vector3( 0, 0, 1 ), 7,6,2,3, points );
    addQuad( Vector3(-1, 0, 0 ), 4,7,3,0, points );
  }
  
  SphereMesh::SphereMesh (int numSegments)
  {
    binding.init( getFormat() );

    //Minimum is 3 segments in XZ plane and
    //half that rounded up vertical segments
    numSegments = Util::Max( numSegments, 3 );
    int numYSegments = (numSegments + 1) / 2;
    
    float xzstep = 2 * PI / numSegments;
    float ystep = PI / numYSegments;
    
    int xzs, ys;
    float xza, ya;
    
    //South pole
    TriVertex sPole = binding( addVertex() );
    *sPole.coord = Vector3( 0.0f, -1.0f, 0.0f );
    *sPole.normal = *sPole.coord;
    
    //Walk half-circle in XY plane bottom-up (6 o'clock up to 12 o'clock)
    for (ys=1, ya = -PI*0.5f + ystep; ys < numYSegments; ++ys, ya += ystep)
    {
      //Cos is radius of the slice in XZ plane
      //Sin is the final Y coordinate
      float r = COS( ya );
      float y = SIN( ya );
      
      //Walk full circle in XZ plane
      for (xzs=0, xza=0.0f; xzs < numSegments; ++xzs, xza+=xzstep)
      {
        //Scale by radius
        float x = COS( xza ) * r;
        float z = SIN( xza ) * r;
        
        //Insert vertex
        TriVertex vert = binding( addVertex() );
        *vert.coord = Vector3( x,y,z );
        *vert.normal = *vert.coord;
      }
    }
    
    //North pole
    TriVertex nPole = binding( addVertex() );
    *nPole.coord = Vector3( 0.0f, 1.0f, 0.0f );
    *nPole.normal = *nPole.coord;

    //Create a group to add faces to
    addFaceGroup( 0 );
    int s, y;

    //South cap
    for (s = 0; s < numSegments; ++s)
    {
      int i1 = 1 + s;
      int i2 = 1 + ((s+1) % numSegments);
      addFace( 0, i2, i1 );
    }

    //Side
    for (y = 0; y < numYSegments-2; ++y )
    {
      for (s = 0; s < numSegments; ++s)
      {
        int i1 = 1 + (y+0) * numSegments + s;
        int i2 = 1 + (y+0) * numSegments + ((s+1) % numSegments);
        int i3 = 1 + (y+1) * numSegments + s;
        int i4 = 1 + (y+1) * numSegments + ((s+1) % numSegments);
        addFace( i1, i2, i3 );
        addFace( i2, i4, i3 );
      }
    }

    //North cap
    for (s = 0; s < numSegments; ++s)
    {
      int i1 = 1 + (numYSegments-2) * numSegments + s;
      int i2 = 1 + (numYSegments-2) * numSegments + ((s+1) % numSegments);
      addFace( i1, i2, (VertexID)data.size()-1 );
    }
  }

}//namespace GE
