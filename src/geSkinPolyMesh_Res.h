#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

namespace GE
{
  /*
  -----------------------------------
  Resource
  -----------------------------------*/

  struct GE_API_ENTRY SkinPolyMeshVertex
  {
    Vector3 point;
    Int32 boneIndex[4];
    Int32 weight[4];
  };

  struct GE_API_ENTRY SkinPolyMeshFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class GE_API_ENTRY SkinPolyMesh
  {
    friend class SkinPolyMesh_Factory;
    
  private:
    Int32                numVerts;
    Int32                numFaces;
    SkinPolyMeshVertex  *verts;
    SkinPolyMeshFace    *faces;
    Int32               *indices;
    
  public:
    SkinPolyMesh ();
  };

  /*
  -----------------------------------
  Factory
  -----------------------------------*/
  
  class GE_API_ENTRY SkinPolyMesh_Factory
  {
  public:
    OCC::ArrayList <SkinPolyMeshVertex>   verts;
    OCC::ArrayList <SkinPolyMeshFace>     faces;
    OCC::ArrayList <Int32>                indices;

    void create (void **outMem, UintP *outSize);
  };

}//namespace GE
#endif//__GESKINPOLYMESH_H
