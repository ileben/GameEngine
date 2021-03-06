#ifndef __GEAXISACTOR_H
#define __GEAXISACTOR_H

#include "util/geUtil.h"
#include "core/geActor.h"
#include "core/geMaterial.h"

namespace GE
{
  class AxisActor : public Actor3D
  {
    virtual void render (MaterialID materialID);
  };
}

#endif//__GEAXISACTOR_H