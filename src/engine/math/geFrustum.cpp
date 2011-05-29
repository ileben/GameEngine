#include <math/geFrustum.h>

namespace GE
{
  void Frustum::fromMatrix (const Matrix4x4 &m)
  {
    planes[ Frustum::Left ]   = m.getRow(3) + m.getRow(0);
    planes[ Frustum::Right ]  = m.getRow(3) - m.getRow(0);
    planes[ Frustum::Bottom ] = m.getRow(3) + m.getRow(1);
    planes[ Frustum::Top ]    = m.getRow(3) - m.getRow(1);
    planes[ Frustum::Near ]   = m.getRow(3) + m.getRow(2);
    planes[ Frustum::Far ]    = m.getRow(3) - m.getRow(2);
  }

  Frustum::Result Frustum::testPoint (const Vector3 &p, int pl) const
  {
    if (p.x  * planes[pl].x +
        p.y  * planes[pl].y +
        p.z  * planes[pl].z +
        1.0f * planes[pl].w > 0.0f)
      return Frustum::Inside;
    else return Frustum::Outside;
  }

  Frustum::Result Frustum::testBox (Vector3 box[8]) const
  {
    //Test against all planes
    for (int p=0; p<6; ++p) {

      int outCount = 0;

      //Test how many points is outside
      for (int b=0; b<8; ++b)
        if (testPoint( box[ b ], p ) == Frustum::Outside)
          outCount++;

      //The box is out if all points are outside any one of the planes
      if (outCount == 8)
        return Frustum::Outside;
    }

    return Frustum::Inside;
  }
}
