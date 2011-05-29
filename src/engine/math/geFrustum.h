#ifndef __GEFRUSTUM_H
#define __GEFRUSTUM_H

#include <math/geVectors.h>
#include <math/geMatrix.h>

namespace GE
{

  class Frustum
  {
  public:

    enum Result
    {
      Outside,
      Inside
    };

    enum PlaneIndex
    {
      Left   = 0,
      Right  = 1,
      Bottom = 2,
      Top    = 3,
      Near   = 4,
      Far    = 5
    };

    Vector4 planes[6];

    void fromMatrix (const Matrix4x4 &m);
    Result testPoint (const Vector3 &p, int pl) const;
    Result testBox (Vector3 box[8]) const;
  };


}//namespace GE
#endif//__GEFRUSTUM_H
