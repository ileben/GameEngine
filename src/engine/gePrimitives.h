#ifndef __GEPRIMITIVES_H
#define __GEPRIMITIVES_H

namespace GE
{
  class CubeMesh : public TriMesh
  {
    DECLARE_SUBCLASS( CubeMesh, TriMesh ); DECLARE_END;
  
  public:
  
    CubeMesh ();
  };
  
  class SphereMesh : public TriMesh
  {
    DECLARE_SUBCLASS( SphereMesh, TriMesh ); DECLARE_END;
    
  public:
    
    SphereMesh (int numSegments=8);
  };
}

#endif//__GEPRIMITIVES_H
