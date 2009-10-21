#ifndef __GEBOUNDINGBOX_H
#define __GEBOUNDINGBOX_H


#include <math/geVectors.h>
#include <math/geMatrix.h>


namespace GE
{

  /*
  --------------------------------------------------
  Bounding box
  --------------------------------------------------*/
  class BoundingBox
  {
  public:
    Vector3 min;
    Vector3 max;

    BoundingBox ();
    BoundingBox (const Vector3 &first);
    BoundingBox& operator = (const Vector3 &first);
    BoundingBox& operator += (const Vector3 &vert);
    BoundingBox& operator += (const BoundingBox &bbox);
    void getCorners (Vector3 *corners);
  };


}//namespace GE
#endif//__GEBOUNDINGBOX_H
