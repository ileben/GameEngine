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

    typedef OCC::ArrayList<Vector3> Vec3Array;
    typedef Vec3Array::Iterator Vec3ArrayIter;

    typedef OCC::ArrayList<UMesh::Vertex*> UVertArray;
    typedef UVertArray::Iterator UVertArrayIter;

    typedef OCC::ArrayList<DMesh::Vertex*> VertArray;
    typedef VertArray::Iterator VertArrayIter;

    typedef OCC::ArrayList<DMesh::SmoothNormal*> SNormalArray;
    typedef SNormalArray::Iterator SNormalArrayIter;

    OCC::FileRef file;
    OCC::ByteString buffer;
    int bufPointer;
    bool endOfFile;

    OCC::ByteString line;
    int linePointer;

    void readLine();
    bool isWhitespace(char c);
    OCC::ByteString parseToken();
    Vector3 parseVector3();

    Vec3Array points;
    Vec3Array ncoords;
    Vec3Array ucoords;
    Uint32 smoothGroup;

    void command_Vertex();
    void command_Normal();
    void command_UVW();
    void command_SmoothGroup();

    Shape *shape;
    UMesh *umesh;
    DMesh *mesh;
    VertArray verts;
    UVertArray uverts;
    SNormalArray snormals;
    void newShape(const OCC::ByteString &name);

    void command_Group();
    void command_Face();

  public:
    bool loadFile(const OCC::String &filename);
  };

}/* namespace GE */
#pragma warning(pop)
#endif /* __GELOADOBJ_H */
