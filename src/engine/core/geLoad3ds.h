#ifndef __GELOAD3DS_H
#define __GELOAD3DS_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "io/geFile.h"
#include "geLoader.h"
#include "gePolyMesh.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*-------------------------------
   * Arrays used by the loader
   *-------------------------------*/

  typedef ArrayList <PolyMesh::Vertex*> VertArray;
  typedef ArrayList <PolyMesh::Face*> FaceArray;
  typedef ArrayList <Vector2> UVArray;

  /*---------------------------------------
   * Stores a single 3ds chunk file layout
   *---------------------------------------*/

  struct ChunkInfo
  {
    int start;
    Int16 id;
    Int32 size;
  };

  /*---------------------------------------
   * Main loader class
   *---------------------------------------*/

  class GE_API_ENTRY Loader3ds : public Loader
  {
    DECLARE_SUBCLASS (Loader3ds, Loader); DECLARE_END;

    //File handling
    File file;
    int fileSize;
    
    //Chunk stack
    ChunkInfo topChunk;
    ArrayList<ChunkInfo> chunkStack;
    void readChunkInfo (ChunkInfo *chunk);
    void pushChunk (const ChunkInfo &chunk);
    void popChunk ();
    bool isChunkDone ();

    //Helpers
    String readString();

    //Chunk processors
    void chunk_EDITOR ();
    void chunk_OBJECT ();
    void chunk_MESH (const String &id);
    void chunk_MESH_VERTEX_LIST (PolyMesh *mesh, VertArray *verts);
    void chunk_MESH_TEX_COORD_LIST (PolyMesh *mesh, UVArray *uvcoords);
    void chunk_MESH_FACE_LIST (PolyMesh *mesh, VertArray *verts, UVArray *uvcoords, FaceArray *faces);
    void chunk_MESH_FACE_SMOOTH_GROUP_LIST (PolyMesh *mesh, FaceArray *faces);
    void chunk_CAMERA (const String &id);
    void chunk_LIGHT (const String &id);
    void chunk_KEYFRAMER ();

  public:
    
    bool loadFile(const String &filename);
  };

}/* namespace GE */
#pragma warning(pop)
#endif /* __GELOAD3DS_H */
