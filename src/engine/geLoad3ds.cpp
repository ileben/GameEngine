#define GE_API_EXPORT
#include "geEngine.h"
#include "geLoad3ds.h"

namespace GE
{
  DEFINE_CLASS (Loader3ds);
  
  enum ChunkType
  {
    CHUNK_NULL                             = 0x0000,
    CHUNK_COLOR_RGB_F                      = 0x0010,
    CHUNK_COLOR_RGB_8                      = 0x0011,
    CHUNK_COLOR_RGB_GAMMA_8                = 0x0012,
    CHUNK_COLOR_RGB_GAMMA_F                = 0x0013,
    CHUNK_PERCENTAGE_16                    = 0x0030,
    CHUNK_PERCENTAGE_F                     = 0x0031,
    CHUNK_MAIN                             = 0x4D4D,
    CHUNK_VERSION                          = 0x0002,
    CHUNK_EDITOR                           = 0x3D3D,
    CHUNK_MASTER_SCALE                     = 0x0100,
    CHUNK_OBJECT                           = 0x4000,
    CHUNK_MESH                             = 0x4100,
    CHUNK_MESH_VERTEX_LIST                 = 0x4110,
    CHUNK_MESH_VERTEX_FLAG_LIST            = 0x4111,
    CHUNK_MESH_FACE_LIST                   = 0x4120,
    CHUNK_MESH_FACE_MATERIAL_LIST          = 0x4130,
    CHUNK_MESH_FACE_SMOOTH_GROUP_LIST      = 0x4150,
    CHUNK_MESH_TEX_COORD_LIST              = 0x4140,
    CHUNK_MESH_MATRIX                      = 0X4165,
    CHUNK_KEYFRAMER                        = 0xB000
  };

  void Loader3ds::pushChunk(const ChunkInfo &chunk)
  {
    //Store last chunk info on stack
    chunkStack.pushBack(topChunk);
    topChunk = chunk;
  }

  void Loader3ds::popChunk()
  {
    //Skip unread chunk data
    int fileCur = file.tell();
    if (fileCur < topChunk.start + topChunk.size)
      file.seek((topChunk.start + topChunk.size) - fileCur, SEEK_CUR);

    //Restore top chunk from stack
    ASSERT(!chunkStack.empty());
    topChunk = chunkStack.last();
    chunkStack.popBack();
  }

  void Loader3ds::readChunkInfo(ChunkInfo *chunk)
  {
    //Save file cursor position and read info
    chunk->start = file.tell();
    if (file.readLE(&chunk->id, 2) != 2) chunk->id = CHUNK_NULL;
    if (file.readLE(&chunk->size, 4) != 4) chunk->size = 0;
  }

  bool Loader3ds::isChunkDone()
  {
    //Return true if file cursor reached chunk end
    int fileCur = file.tell();
    if (fileCur == -1) return true;
    return (fileCur >= topChunk.start + topChunk.size) ||
           (fileCur >= fileSize);
  }

  String Loader3ds::readString()
  {
    //Get file cursor position
    String str; Int8 ch = -1;
    int fileCur = file.tell();
    if (fileCur == -1) return str;

    //Read chars until end of chunk
    while (!isChunkDone()) {

      //Stop on file or string end
      if (file.read(&ch, 1) == 0) break;
      if (ch == 0) break;
      str += ch; fileCur++;
    }

    return str;
  }

  void Loader3ds::chunk_MESH_FACE_SMOOTH_GROUP_LIST (PolyMesh *mesh, FaceArray *faces)
  {
    //Just read all smoothgroups and apply to faces
    for (UintSize i=0; i<faces->size(); ++i) {
      Int32 smoothGroups = 0;
      if (file.readLE(&smoothGroups, 4) != 4) break;
      faces->elementAt(i)->smoothGroups = smoothGroups;
    }
  }

  void Loader3ds::chunk_MESH_FACE_LIST (PolyMesh *mesh, VertArray *verts,
                                        UVArray *uvcoords, FaceArray *faces)
  {
    //Read number of faces
    Int16 faceCount = 0;
    if (file.readLE(&faceCount, 2) != 2)
      return;

    for (int i=0; i<faceCount; ++i) {

      //Read vertex indices
      Int16 i1, i2, i3;
      Int16 flag;
      if (file.readLE(&i1, 2) != 2) return;
      if (file.readLE(&i2, 2) != 2) return;
      if (file.readLE(&i3, 2) != 2) return;
      if (file.readLE(&flag, 2) != 2) return;

      //Make sure indices are in vertex range
      if (i1 < 0 || i1 >= (Int16) verts->size()) return;
      if (i2 < 0 || i2 >= (Int16) verts->size()) return;
      if (i3 < 0 || i3 >= (Int16) verts->size()) return;

      //Add face to mesh
      PolyMesh::Vertex* v[3] = {
        verts->elementAt(i1),
        verts->elementAt(i2),
        verts->elementAt(i3) };
      PolyMesh::Face* face = (PolyMesh::Face*)mesh->addFace((HMesh::Vertex**)v, 3);
      faces->pushBack(face);

      //Make sure indices are in UV range
      if (i1 >= (Int16) uvcoords->size()) continue;
      if (i2 >= (Int16) uvcoords->size()) continue;
      if (i3 >= (Int16) uvcoords->size()) continue;

      //Apply UV coordinates
      //TODO: Add UV face instead
      //DMesh::HalfEdge *h = face->hedgeTo(v[0]);
      //if (h == NULL) return;
      //h->uv = uvcoords->elementAt(i1);
      //h->nextHedge()->uv = uvcoords->elementAt(i2);
      //h->nextHedge()->nextHedge()->uv = uvcoords->elementAt(i3);
    }

    //Process sub-chunks
    while (!isChunkDone()) {

      ChunkInfo subChunk;
      readChunkInfo(&subChunk);
      pushChunk(subChunk);

      switch(subChunk.id) {
      case CHUNK_MESH_FACE_SMOOTH_GROUP_LIST:
        chunk_MESH_FACE_SMOOTH_GROUP_LIST(mesh, faces);
        break;
      default:;
      }

      popChunk();
    }
  }

  void Loader3ds::chunk_MESH_TEX_COORD_LIST (PolyMesh *mesh, UVArray *uvcoords)
  {
    //Read number of UV vertices
    Int16 pointCount;
    if (file.readLE(&pointCount, 2) != 2) return;
    uvcoords->reserve(pointCount);

    for (int i=0; i<pointCount; ++i) {

      //Read UV coords
      Float32 u,v;
      if (file.readLE(&u, 4) != 4) break;
      if (file.readLE(&v, 4) != 4) break;

      //Add to temp array
      uvcoords->pushBack(Vector2(u,1.0f-v));
    }
  }

  void Loader3ds::chunk_MESH_VERTEX_LIST (PolyMesh *mesh, VertArray *verts)
  {
    //Read number of vertices
    Int16 pointCount = 0;
    if (file.readLE(&pointCount, 2) != 2)
      return;

    for (int i=0; i<pointCount; ++i) {

      //Read coords for vertex
      Float32 x,y,z;
      if (file.readLE(&x, 4) != 4) break;
      if (file.readLE(&y, 4) != 4) break;
      if (file.readLE(&z, 4) != 4) break;

      //Add to mesh
      PolyMesh::Vertex* vert = (PolyMesh::Vertex*)mesh->addVertex();
      vert->point.set(-x,z,y);
      verts->pushBack( vert );
    }
  }

  void Loader3ds::chunk_MESH (const String &id)
  { 
    //Create new mesh resource
    PolyMesh *mesh = new PolyMesh;
    resources.pushBack(mesh);
    
    //Create new shape object
    PolyMeshActor *shape = new PolyMeshActor;
    shape->setId (id);
    shape->setMesh (mesh);
    objects.pushBack (shape);
    root->addChild (shape);

    //Temporary lists
    VertArray verts;
    FaceArray faces;
    UVArray uvcoords;

    //Process sub-chunks
    while (!isChunkDone()) {

      ChunkInfo subChunk;
      readChunkInfo(&subChunk);
      pushChunk(subChunk);

      switch(subChunk.id) {
      case CHUNK_MESH_VERTEX_LIST:
        chunk_MESH_VERTEX_LIST (mesh, &verts);
        break;
      case CHUNK_MESH_TEX_COORD_LIST:
        chunk_MESH_TEX_COORD_LIST (mesh, &uvcoords);
        break;
      case CHUNK_MESH_FACE_LIST:
        chunk_MESH_FACE_LIST (mesh, &verts, &uvcoords, &faces);
        break;
      default:;
      }

      popChunk();
    }

    //Apply tex coords to faces
    for (UintSize f=0; f<faces.size(); ++f) {

    }
  }

  void Loader3ds::chunk_OBJECT ()
  {
    //get object name
    String id = readString();

    //Proccess sub-chunks
    while (!isChunkDone()) {

      ChunkInfo subChunk;
      readChunkInfo(&subChunk);
      pushChunk(subChunk);

      switch(subChunk.id) {
      case CHUNK_MESH:
        chunk_MESH(id);
        break;
      default:;
      }

      popChunk();
    }   
  }

  void Loader3ds::chunk_EDITOR ()
  {
    //Process sub-chukns
    while(!isChunkDone()) {

      ChunkInfo subChunk;
      readChunkInfo(&subChunk);
      pushChunk(subChunk);

      switch(subChunk.id) {
      case CHUNK_OBJECT:
        chunk_OBJECT();
        break;
      default:;
      }

      popChunk();
    }
  }

  void Loader3ds::chunk_KEYFRAMER ()
  {
  }

  bool Loader3ds::loadFile( const String &filename )
  {
    //Try to open file
    File module = File::GetModule();
    file = module.getRelativeFile( filename );
    fileSize = file.getSize();
    if( !file.open( "rb" )) return false;

    //Read main chunk info
    ChunkInfo mainChunk;
    readChunkInfo( &mainChunk );
    if( mainChunk.id != CHUNK_MAIN )
      return false;
    pushChunk( mainChunk );

    //Create root object node
    root = new Actor();

    //Process sub-chunks
    while( !isChunkDone() )
    {
      ChunkInfo subChunk;
      readChunkInfo( &subChunk );
      pushChunk( subChunk );

      switch( subChunk.id ){
      case CHUNK_EDITOR:
        chunk_EDITOR();
        break;
      case CHUNK_KEYFRAMER:
        chunk_KEYFRAMER();
        break;
      default:;
      }

      popChunk();
    }
    
    //End main chunk
    popChunk();
    file.close();
    return true;
  }

}/* namespace GE */
