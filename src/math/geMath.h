#ifndef __GEMATH_H
#define __GEMATH_H

#include "math/geVectors.h"
#include "math/geMatrix.h"

/*
--------------------------------------------------
Bounding box
--------------------------------------------------*/
namespace GE
{
  struct BoundingBox
  {
    Vector3 min;
    Vector3 max;
    Vector3 center;
  };
}

#endif//__GEMATH_H
