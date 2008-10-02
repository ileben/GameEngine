#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Saver);
  DEFINE_CLASS (SaverObj);
  
  void Saver::addObject (Object *o)
  {
    objects.pushBack(o);
  }

  void SaverObj::writeDynMesh (PolyMesh *mesh, UMesh *umesh)
  {
    int id = 0;
    int normalCount = 0;
    PolyMesh::VertIter v;

    //Output and index vertices
    for (v.begin(mesh); !v.end(); ++v) {
      file->write( ByteString::Format(
        "v %f %f %f\n", v->point.x, v->point.y, v->point.z) );
      v->tag.id = ++id; }

    //Total no. of vertices
    file->write( ByteString::Format("# %d vertices\n\n", id) );
    

    //Output and index UV vertices
    id = 0; for (UMesh::VertIter u(umesh); !u.end(); ++u) {
      file->write( ByteString::Format(
        "vt %f %f %f\n", u->point.x, 1.0f-u->point.y, 1.0f) );
      u->tag.id = ++id; }

    //Total no. of UV vertices
    file->write( ByteString::Format("# %d texture vertices\n\n", id) );
    
    
    //Output and index vertex normals
    id =0; for (PolyMesh::SmoothNormalIter s(mesh); !s.end(); ++s) {
      file->write( ByteString::Format(
        "vn %f %f %f\n", s->coord.x, s->coord.y, s->coord.z));
      s->tag.id = ++id; }

    //Total no. of unique normals
    file->write( ByteString::Format("# %d normals\n\n", id) );
    
    
    //Output faces
    //(we walk uv mesh in parallel with vertex mesh. In case
    //the uv mesh is incompatible at any point, the uv index
    //will simply not be exported)
    
    Uint32 lastSmoothGroups = 0;
    UMesh::FaceIter uf(umesh);
    for (PolyMesh::FaceIter f(mesh); !f.end(); ++f, ++uf) {
    
      if (f->smoothGroups != lastSmoothGroups) {
        file->write( ByteString::Format("s %d\n", f->smoothGroups) );
        lastSmoothGroups = f->smoothGroups; }
      
      file->write("f");
    
      UMesh::FaceHedgeIter uh(*uf);
      for (PolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uh) {
      
        if (!uh.end()) {
        
          file->write( ByteString::Format(
            " %d/%d/%d", h->dstVertex()->tag.id, uh->dstVertex()->tag.id,
                         h->smoothNormal()->tag.id) );
        }else{
        
          file->write( ByteString::Format(
            " %d//%d", h->dstVertex()->tag.id, h->smoothNormal()->tag.id) ); }
      }
      
      file->write("\n");
    }
  }

  void SaverObj::writeShape(PolyMeshActor *shape)
  {
    //Group name
    file->write("g ");
    file->write(shape->id);
    file->write("\n");
    
    //Dynamic mesh
    if (shape->polyMesh != NULL)
      writeDynMesh (shape->polyMesh, shape->texMesh);
  }

  bool SaverObj::saveFile(const String &filename)
  {
    //Try to open file
    FileRef module = File::GetModule();
    file = module->getRelativeFile (filename);
    if (!file->open("wb")) return false;
    
    //Walk objects to save
    for (int i=0; i<objects.size(); ++i) {
      if (ClassOf (objects[i]) == Class (PolyMeshActor)) {
        writeShape ((PolyMeshActor*) objects[i]); }
    }

    file->close();
    return true;
  }

}/* namespace GE */
