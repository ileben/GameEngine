#ifndef __GELOAD3DS_H
#define __GELOAD3DS_H
#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*-------------------------------
   * Arrays used by the loader
   *-------------------------------*/

  typedef OCC::ArrayList <PolyMesh::Vertex*> VertArray;
  typedef VertArray::Iterator VertArrayIter;

  typedef OCC::ArrayList <PolyMesh::Face*> FaceArray;
  typedef FaceArray::Iterator FaceArrayIter;

  typedef OCC::ArrayList <Vector2> UVArray;

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
    OCC::FileRef file;
    int fileSize;
    
    //Chunk stack
    ChunkInfo topChunk;
    OCC::ArrayList<ChunkInfo> chunkStack;
    void readChunkInfo (ChunkInfo *chunk);
    void pushChunk (const ChunkInfo &chunk);
    void popChunk ();
    bool isChunkDone ();

    //Helpers
    OCC::String readString();

    //Chunk processors
    void chunk_EDITOR ();
    void chunk_OBJECT ();
    void chunk_MESH (const OCC::String &id);
    void chunk_MESH_VERTEX_LIST (PolyMesh *mesh, VertArray *verts);
    void chunk_MESH_TEX_COORD_LIST (PolyMesh *mesh, UVArray *uvcoords);
    void chunk_MESH_FACE_LIST (PolyMesh *mesh, VertArray *verts, UVArray *uvcoords, FaceArray *faces);
    void chunk_MESH_FACE_SMOOTH_GROUP_LIST (PolyMesh *mesh, FaceArray *faces);
    void chunk_CAMERA (const OCC::String &id);
    void chunk_LIGHT (const OCC::String &id);
    void chunk_KEYFRAMER ();

  public:
    
    bool loadFile(const OCC::String &filename);
  };

}/* namespace GE */
#pragma warning(pop)
#endif /* __GELOAD3DS_H */
