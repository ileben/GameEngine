class VertIter
{
private:
  HMesh *mesh;
  ListHandle it;
  
public:

  VertIter() {
    mesh = NULL;
  }

  VertIter(HMesh *mesh) {
    begin(mesh);
  };
  
  VertIter(HMesh *mesh, const ListHandle &it) {
    begin(mesh, it);
  }

  void begin(HMesh *mesh) {
    this->mesh = mesh;
    if (mesh != NULL)
      this->it = mesh->verts.begin();
  }

  void begin(HMesh *mesh, const ListHandle &it) {
    this->mesh = mesh;
    this->it = it;
  }
  
  VertIter& operator++() {
    if (mesh != NULL)
      if (it != mesh->verts.end())
        ++it;
    return *this;
  }

  bool end() {
    if (mesh == NULL) return true;
    return (it == mesh->verts.end());
  }

  Vertex* operator*() const {
    if (mesh != NULL)
      if (it != mesh->verts.end())
        return (Vertex*)*it;
    return NULL;
  }

  Vertex* operator->() const {
    return (Vertex*)*it;
  }
};


class HedgeIter
{
private:
  HMesh *mesh;
  ListHandle it;
  
public:

  HedgeIter() {
    this->mesh = NULL;
  }

  HedgeIter(HMesh *mesh) {
    begin(mesh);
  };
  
  HedgeIter(HMesh *mesh, const ListHandle &it) {
    begin(mesh, it);
  }

  void begin(HMesh *mesh) {
    this->mesh = mesh;
    if (mesh != NULL)
      this->it = mesh->hedges.begin();
  }

  void begin(HMesh *mesh, const ListHandle &it) {
    this->mesh = mesh;
    this->it = it;
  }
  
  HedgeIter& operator++() {
    if (mesh != NULL)
      if (it != mesh->hedges.end())
        ++it;
    return *this;
  }

  bool end() {
    if (mesh == NULL) return true;
    return (it == mesh->hedges.end());
  }

  HalfEdge* operator*() const {
    if (mesh != NULL)
      if (it != mesh->hedges.end())
        return (HalfEdge*)*it;
    return NULL;
  }

  HalfEdge* operator->() const {
    return (HalfEdge*)*it;
  }
};


class EdgeIter
{
private:
  HMesh *mesh;
  ListHandle it;
  
public:

  EdgeIter() {
    this->mesh = NULL;
  }

  EdgeIter(HMesh *mesh) {
    begin(mesh);
  }
  
  EdgeIter(HMesh *mesh, const ListHandle &it) {
    begin(mesh, it);
  }

  void begin(HMesh *mesh) {
    this->mesh = mesh;
    if (mesh != NULL)
      this->it = mesh->edges.begin();
  }

  void begin(HMesh *mesh, const ListHandle &it) {
    this->mesh = mesh;
    this->it = it;
  }
  
  EdgeIter& operator++() {
    if (mesh != NULL)
      if (it != mesh->edges.end())
        ++it;
    return *this;
  }

  bool end() {
    if (mesh == NULL) return true;
    return (it == mesh->edges.end());
  }

  Edge* operator*() const {
    if (mesh != NULL)
      if (it != mesh->edges.end())
        return (Edge*)*it;
    return NULL;
  }

  Edge* operator->() const {
    return (Edge*)*it;
  }
};


class FaceIter
{
private:
  HMesh *mesh;
  ListHandle it;
  
public:

  FaceIter() {
    this->mesh = NULL;
  };

  FaceIter(HMesh *mesh) {
    begin(mesh);
  };
  
  FaceIter(HMesh *mesh, const ListHandle &it) {
    begin(mesh, it);
  }

  void begin(HMesh *mesh) {
    this->mesh = mesh;
    if (mesh != NULL)
      this->it = mesh->faces.begin();
  };
  
  void begin(HMesh *mesh, const ListHandle &it) {
    this->mesh = mesh;
    this->it = it;
  }
  
  FaceIter& operator++() {
    if (mesh != NULL)
      if (it != mesh->faces.end())
        ++it;
    return *this;
  }

  bool end() {
    if (mesh == NULL) return true;
    return (it == mesh->faces.end());
  }

  Face* operator*() const {
    if (mesh != NULL)
      if (it != mesh->faces.end())
        return (Face*)*it;
    return NULL;
  }

  Face* operator->() const {
    return (Face*)*it;
  }
};
