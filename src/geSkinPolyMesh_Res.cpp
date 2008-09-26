#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{
  void SkinPolyMesh_Factory::create (void **outMem, UintP *outSize)
  {
    //Calculate sizes
    UintP structSize  = sizeof (SkinPolyMesh);
    UintP vertsSize   = verts.size() * sizeof (SkinPolyMeshVertex*);
    UintP facesSize   = faces.size() * sizeof (SkinPolyMeshFace*);
    UintP indicesSize = indices.size() * sizeof (SkinPolyMeshFace*);
    UintP dataSize = structSize + vertsSize + facesSize + indicesSize;
    *outSize = dataSize;
    
    //Allocate memory
    SkinPolyMesh *data = (SkinPolyMesh*) std::malloc (dataSize);
    *outMem = data;
    
    //Adjust pointers
    Util::PtrSet (&data->verts,   (UintP)data + structSize);
    Util::PtrSet (&data->faces,   (UintP)data + structSize + vertsSize);
    Util::PtrSet (&data->indices, (UintP)data + structSize + vertsSize + facesSize);
    
    //Copy data
    data->numVerts = verts.size();
    data->numFaces = faces.size();
    std::memcpy (data->verts, verts.buffer(), vertsSize);
    std::memcpy (data->faces, faces.buffer(), facesSize);
    std::memcpy (data->indices, indices.buffer(), indicesSize);
    
    //Re-adjust pointers
    Util::PtrSub (&data->verts,   (UintP)data);
    Util::PtrSub (&data->faces,   (UintP)data);
    Util::PtrSub (&data->indices, (UintP)data);
  }
  
  SkinPolyMesh::SkinPolyMesh ()
  {
    Util::PtrAdd (&verts,   (UintP)this);
    Util::PtrAdd (&faces,   (UintP)this);
    Util::PtrAdd (&indices, (UintP)this);
  }

}//namespace GE
