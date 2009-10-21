#include <math/geBoundingBox.h>

namespace GE
{
  BoundingBox::BoundingBox ()
  {
    min = max = Vector3( 0,0,0 );
  }

  BoundingBox::BoundingBox (const Vector3 &first)
  {
    min = max = Vector3( 0,0,0 );
  }

  BoundingBox& BoundingBox::operator = (const Vector3 &first)
  {
    min = max = Vector3( 0,0,0 );
    return *this;
  }

  BoundingBox& BoundingBox::operator += (const Vector3 &vert)
  {
    if (vert.x < min.x) min.x = vert.x;
    if (vert.y < min.y) min.y = vert.y;
    if (vert.z < min.z) min.z = vert.z;

    if (vert.x > max.x) max.x = vert.x;
    if (vert.y > max.y) max.y = vert.y;
    if (vert.z > max.z) max.z = vert.z;

    return *this;
  }

  BoundingBox& BoundingBox::operator += (const BoundingBox &bbox)
  {      
    if (bbox.min.x < min.x) min.x = bbox.min.x;
    if (bbox.min.y < min.y) min.y = bbox.min.y;
    if (bbox.min.z < min.z) min.z = bbox.min.z;

    if (bbox.max.x > max.x) max.x = bbox.max.x;
    if (bbox.max.y > max.y) max.y = bbox.max.y;
    if (bbox.max.z > max.z) max.z = bbox.max.z;

    return *this;
  }

  void BoundingBox::getCorners (Vector3 *corners)
  {
    corners[0].set( min.x, min.y, min.z );
    corners[1].set( min.x, min.y, max.z );
    corners[2].set( min.x, max.y, min.z );
    corners[3].set( min.x, max.y, max.z );
    corners[4].set( max.x, min.y, min.z );
    corners[5].set( max.x, min.y, max.z );
    corners[6].set( max.x, max.y, min.z );
    corners[7].set( max.x, max.y, max.z );
  }
}
