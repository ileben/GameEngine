#ifndef __GEPRIMITIVES_H
#define __GEPRIMITIVES_H

namespace GE
{
  class CubeMesh : public TriMesh
  {
    DECLARE_SUBCLASS( CubeMesh, TriMesh ); DECLARE_END;

  private:

    void addQuad (const Vector3 &normal,
                  int i1, int i2, int i3, int i4,
                  Vector3 *points);
  
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
