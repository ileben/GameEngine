#ifndef __GEPRIMITIVES_H
#define __GEPRIMITIVES_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "geTriMesh.h"

namespace GE
{
  class CubeMesh : public TriMesh
  {
    CLASS( CubeMesh, TriMesh,
      793c7557,40c5,41ae,a186ff82879d24b2 );

  private:

    void addQuad (const Vector3 &normal,
                  int i1, int i2, int i3, int i4,
                  Vector3 *points);
  
  public:
  
    CubeMesh ();
  };
  
  class SphereMesh : public TriMesh
  {
    CLASS( SphereMesh, TriMesh,
      cac5cc04,6632,49bf,9dc19bf7e3a7b232 );
    
  public:
    
    SphereMesh (int numSegments=8);
  };
}

#endif//__GEPRIMITIVES_H
