#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Shape);
  
  Shape::Shape()
  {
    uvMesh = NULL;
    statMesh = NULL;
    dynMesh = NULL;
    material = NULL;
    useDynamic = true;
  }

  Shape::~Shape()
  {
    if (uvMesh != NULL)
      uvMesh->dereference();
    if (statMesh != NULL)
      statMesh->dereference();
    if (dynMesh != NULL)
      dynMesh->dereference();
    if (material != NULL)
      material->dereference();
  }

  void Shape::setUV(UMesh *mesh)
  {
    if (uvMesh != NULL)
      uvMesh->dereference();

    uvMesh = mesh;
    uvMesh->reference();
  }

  UMesh* Shape::getUV() {
    return uvMesh;
  }

  void Shape::setStatic(SMesh *mesh)
  {
    if (statMesh != NULL)
      statMesh->dereference();

    statMesh = mesh;
    statMesh->reference();
    useDynamic = false;
  }

  SMesh* Shape::getStatic() {
    return statMesh;
  }

  void Shape::setDynamic(DMesh *mesh)
  {
    if (dynMesh != NULL)
      dynMesh->dereference();

    dynMesh = mesh;
    dynMesh->reference();
    useDynamic = true;
  }

  DMesh* Shape::getDynamic() {
    return dynMesh;
  }

  void Shape::setMaterial(Material *mat)
  {
    if (material != NULL)
      material->dereference();

    material = mat;
    material->reference();
  }

  Material* Shape::getMaterial() {
    return material;
  }
}
