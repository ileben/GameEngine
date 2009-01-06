#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{
  DEFINE_CLASS( CubeMesh );
  DEFINE_CLASS( SphereMesh );
  

  CubeMesh::CubeMesh ()
  {
    
  }
  
  SphereMesh::SphereMesh (int numSegments)
  {
    //Minimum is 3 segments in XZ plane and
    //half that rounded up vertical segments
    numSegments = Util::Max( numSegments, 3 );
    int numYSegments = (numSegments + 1) / 2;
    
    float xzstep = 2 * PI / numSegments;
    float ystep = PI / numYSegments;
    
    int xzs, ys;
    float xza, ya;
    
    //South pole
    TriMeshVertex sPole;
    sPole.point = Vector3( 0.0f, -1.0f, 0.0f );
    sPole.normal = sPole.point;
    this->addVertex( &sPole );
    
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
        TriMeshVertex vert;
        vert.point = Vector3( x,y,z );
        vert.normal = vert.point;
        this->addVertex( &vert );
      }
    }
    
    //North pole
    TriMeshVertex nPole;
    nPole.point = Vector3( 0.0f, 1.0f, 0.0f );
    nPole.normal = nPole.point;
    this->addVertex( &nPole );
  }

}//namespace GE
