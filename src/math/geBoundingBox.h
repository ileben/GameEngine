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
    Vector3 center;

    BoundingBox ();
    void getCorners (Vector3 *corners);
    BoundingBox& operator += (const BoundingBox &bbox);
  };


}//namespace GE
#endif//__GEBOUNDINGBOX_H
