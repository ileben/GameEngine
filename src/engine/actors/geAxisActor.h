#ifndef __GEAXISACTOR_H
#define __GEAXISACTOR_H

#include "util/geUtil.h"
#include "engine/geActor.h"
#include "engine/geMaterial.h"

namespace GE
{
  class AxisActor : public Actor
  {
    virtual void render (MaterialID materialID);
  };
}

#endif//__GEAXISACTOR_H