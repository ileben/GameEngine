#ifndef __GELOADOBJ_H
#define __GELOADOBJ_H
#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{

/*-----------------------------------------
   * Main loader class
   *---------------------------------------*/

  class GE_API_ENTRY LoaderObj : public Loader
  {
    DECLARE_SUBCLASS (LoaderObj, Loader); DECLARE_END;

    typedef ArrayList <Vector3> Vec3Array;
    typedef ArrayList <TexMesh::Vertex*> UVertArray;
    typedef ArrayList <PolyMesh::Vertex*> VertArray;
    typedef ArrayList <PolyMesh::VertexNormal*> VNormalArray;

    File file;
    ByteString buffer;
    int bufPointer;
    bool endOfFile;

    ByteString line;
    int linePointer;

    void readLine();
    bool isWhitespace(char c);
    ByteString parseToken();
    Vector3 parseVector3();

    Vec3Array points;
    Vec3Array ncoords;
    Vec3Array ucoords;
    Uint32 smoothGroup;

    void command_Vertex();
    void command_Normal();
    void command_UVW();
    void command_SmoothGroup();

    TexMesh *umesh;
    PolyMesh *mesh;
    PolyMeshActor *shape;
    VertArray verts;
    UVertArray uverts;
    VNormalArray vnormals;
    void newShape(const ByteString &name);

    void command_Group();
    void command_Face();

  public:
    bool loadFile(const String &filename);
  };

}/* namespace GE */
#pragma warning(pop)
#endif /* __GELOADOBJ_H */
