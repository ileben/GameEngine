/*=============================================
 *
 * Walks smooth groups of faces adjacent to
 * given vertex
 *
 *=============================================*/

class VertexNormalIter
{
  PolyMesh *mesh;
  int cur;

public:

  VertexNormalIter()
  {
    mesh = NULL;
    cur = 0;
  }

  VertexNormalIter (PolyMesh *mesh)
  {
    begin (mesh);
  }

  void begin (PolyMesh *mesh)
  {
    this->mesh = mesh;
    cur = 0;
  }

  VertexNormalIter& operator++ ()
  {
    cur++;
    return *this;
  }

  bool end() const
  {
    if (mesh == NULL) return true;
    return (cur >= mesh->vertexNormals.size());
  }

  VertexNormal* operator*() const {
    return (mesh == NULL) ? NULL : &mesh->vertexNormals [cur];
  }

  VertexNormal* operator->() const {
    return (mesh == NULL) ? NULL : &mesh->vertexNormals [cur];
  }
};

/*=============================================
 *
 * Walk faces of a certain material id
 *
 *=============================================*/

class MaterialFaceIter
{
  PolyMesh *mesh;
  ListHandle cur;
  Uint8 materialID;
  
public:
  
  MaterialFaceIter (PolyMesh *mesh, Uint8 materialID)
  {
    begin( mesh, materialID );
  }
  
  MaterialFaceIter (const MaterialFaceIter &it)
  {
    begin( it );
  }
  
  void begin (PolyMesh *mesh, Uint8 materialID)
  {
    this->mesh = mesh;
    this->materialID = materialID;
    if (mesh == NULL) return;
    
    cur = mesh->faces.begin();
    if (((Face*)*cur)->materialID() != materialID)
      ++(*this);
  }
  
  void begin (const MaterialFaceIter &it)
  {
    mesh = it.mesh;
    cur = it.cur;
    materialID = it.materialID;
  }
  
  MaterialFaceIter& operator++()
  {
    if (mesh == NULL) return *this;
    if (cur == mesh->faces.end()) return *this;

    for (++cur; cur!=mesh->faces.end(); ++cur)
      if (((Face*)*cur)->materialID() == materialID)
        break;
    
    return *this;
  }
  
  bool end() const
  {
    return (mesh == NULL ? true : cur == mesh->faces.end());
  }
  
  Face* operator*() const
  {
    if (mesh != NULL)
      if (cur != mesh->faces.end())
        return (Face*)(*cur);
    return NULL;
  }
  
  Face* operator->() const
  {
    return (Face*)(*cur);
  }
};


class FaceTriIter
{
private:
  
  Triangle *cur;

public:

  FaceTriIter ()
  {
    cur = NULL;
  }

  FaceTriIter (Face *face)
  {
    begin( face );
  }

  void begin (Face *face)
  {
    cur = face->firstTriangle();
  }

  FaceTriIter& operator++()
  {
    if (cur != NULL)
      cur = cur->nextTriangle();
    return *this;
  }

  bool end() const
  {
    return (cur == NULL);
  }

  Triangle* operator*() const
  {
    return cur;
  }

  Triangle* operator->() const
  {
    return cur;
  }
};
