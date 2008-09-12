
/*=============================================
 *
 * Walks all half-edges outgoing or incoming
 * to / from given vertex
 *
 *=============================================*/

class VertHedgeIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  bool twin;
  
public:

  VertHedgeIter()
  {
    start = NULL;
    cur = NULL;
    twin = true;
    moved = false;
  }

  VertHedgeIter(Vertex *vert) {
    begin(vert);
  }

  VertHedgeIter(const VertHedgeIter &it) {
    begin(it);
  }

  VertHedgeIter(Vertex *vert, HalfEdge *hedge) {
    begin(vert, hedge);
  }

  void begin(Vertex *vert)
  {
    if (vert == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = vert->hedge;
      cur = vert->hedge;
      twin = true;
      moved = false;
    }
  }

  void begin(const VertHedgeIter &it)
  {
    start = it.cur;
    cur = it.cur;
    twin = it.twin;
    moved = false;
  }

  void begin(Vertex *vert, HalfEdge *hedge)
  {
    begin(vert);

    while (*(*this)!=hedge && !end())
      ++(*this);
    
    start = cur;
    moved = false;
  }
  
  VertHedgeIter& operator++()
  {
    if (cur == NULL)
      return *this;
    
    if (twin)
      cur = cur->twin;
    else
      cur = cur->next;
    
    twin = !twin;
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  HalfEdge* operator*() const {
    return (HalfEdge*)cur;
  }

  HalfEdge* operator->() const {
    return (HalfEdge*)cur;
  }
};

class VertHedgeBackIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  bool twin;
  
public:

  VertHedgeBackIter()
  {
    start = NULL;
    cur = NULL;
    twin = false;
    moved = false;
  }

  VertHedgeBackIter(Vertex *vert) {
    begin(vert);
  }

  VertHedgeBackIter(const VertHedgeBackIter &it) {
    begin(it);
  }

  VertHedgeBackIter(Vertex *vert, HalfEdge *hedge) {
    begin(vert, hedge);
  }

  void begin(Vertex *vert)
  {
    if (vert == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = vert->hedge;
      cur = vert->hedge;
      twin = false;
      moved = false;
    }
  }

  void begin(const VertHedgeBackIter &it)
  {
    start = it.cur;
    cur = it.cur;
    twin = it.twin;
    moved = false;
  }

  void begin(Vertex *vert, HalfEdge *hedge)
  {
    begin(vert);

    while (*(*this)!=hedge && !end())
      ++(*this);
    
    start = cur;
    moved = false;
  }
  
  VertHedgeBackIter& operator++()
  {
    if (cur == NULL)
      return *this;
    
    if (twin)
      cur = cur->twin;
    else
      cur = cur->prev;
    
    twin = !twin;
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  HalfEdge* operator*() const {
    return (HalfEdge*)cur;
  }

  HalfEdge* operator->() const {
    return (HalfEdge*)cur;
  }
};

/*==============================================
 *
 * Walks all vertices around given vertex which
 * are immediatelly connected to it
 *
 *==============================================*/

class VertVertIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  
public:

  VertVertIter()
  {
    start = NULL;
    cur = NULL;
    moved = false;
  }

  VertVertIter(Vertex *vert) {
    begin(vert);
  }

  VertVertIter(const VertVertIter &it) {
    begin(it);
  }

  VertVertIter(Vertex *vert, Vertex *vert2) {
    begin(vert, vert2);
  }

  void begin(Vertex *vert)
  {
    if (vert == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = vert->hedge;
      cur = vert->hedge;
      moved = false;
    }
  }

  void begin(const VertVertIter &it)
  {
    start = it.cur;
    cur = it.cur;
    moved = false;
  }

  void begin(Vertex *vert, Vertex *vert2)
  {
    begin(vert);

    while (*(*this)!=vert2 && !end())
      ++(*this);

    start = cur;
    moved = false;
  }
  
  VertVertIter& operator++()
  {  
    if (cur == NULL)
      return *this;
    
    cur = cur->twin;
    cur = cur->next;
    
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  Vertex* operator*() const {
    return (Vertex*)(cur == NULL ? NULL : cur->vert);
  }

  Vertex* operator->() const {
    return (Vertex*)(cur->vert);
  }
};

/*================================================
 *
 * Walks all faces around and containing the
 * given vertex
 *
 *==============================================*/

class VertFaceIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  
public:

  VertFaceIter()
  {
    start = NULL;
    cur = NULL;
    moved = false;
  }

  VertFaceIter(Vertex *vert) {
    begin(vert);
  }

  VertFaceIter(const VertFaceIter &it) {
    begin(it);
  }

  VertFaceIter(Vertex *vert, Face *face) {
    begin(vert, face);
  }
  
  void begin(Vertex *vert)
  {
    if (vert == NULL) {
      start = NULL;
      cur = NULL;
    }else{

      start = vert->hedge;
      cur = vert->hedge;
      moved = false;

      if (cur != NULL)
        if (cur->face == NULL)
          ++(*this);
    }
  }

  void begin(const VertFaceIter &it)
  {
    start = it.cur;
    cur = it.cur;
    moved = false;
  }

  void begin(Vertex *vert, Face *face)
  {
    begin(vert);

    while (*(*this)!=face && !end())
      ++(*this);

    start = cur;
    moved = false;
  }
  
  VertFaceIter& operator++()
  {
    if (cur == NULL)
      return *this;
    
    do {
      
      cur = cur->twin;
      cur = cur->next;
      moved = true;
      if(cur == start) break;
      
    } while(cur->face == NULL);

    return *this;
  }
  
  bool end() const {  
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  Face* operator*() const {
    return (Face*)(cur == NULL ? NULL : cur->face);
  }

  Face* operator->() const {
    return (Face*)(cur->face);
  }

  HalfEdge* hedgeToVertex() const {
    return (HalfEdge*)cur->prev;
  }
};

/*================================================
 *
 * Walks all half-edges in the internal loop
 * of the given face
 *
 *==============================================*/

class FaceHedgeIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  
public:

  FaceHedgeIter()
  {
    start = NULL;
    cur = NULL;
  }

  FaceHedgeIter(Face *face) {
    begin(face);
  }
  
  FaceHedgeIter(const FaceHedgeIter &it) {
    begin(it);
  }

  void begin(Face *face)
  {
    if (face == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = face->hedge;
      cur = face->hedge;
    }
    moved = false;
  }

  void begin(const FaceHedgeIter &it)
  {
    start = it.start;
    cur = it.cur;
    moved = false;
  }
  
  FaceHedgeIter& operator++()
  {  
    if (cur == NULL)
      return *this;
    
    cur = cur->next;
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  HalfEdge* operator*() const {
    return (HalfEdge*)cur;
  }

  HalfEdge* operator->() const {
    return (HalfEdge*)cur;
  }
};

class FaceVertIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  
public:

  FaceVertIter()
  {
    start = NULL;
    cur = NULL;
  }

  FaceVertIter(Face *face) {
    begin(face);
  }
  
  FaceVertIter(const FaceVertIter &it) {
    begin(it);
  }

  void begin(Face *face)
  {
    if (face == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = face->hedge;
      cur = face->hedge;
    }
    moved = false;
  }

  void begin(const FaceVertIter &it)
  {
    start = it.start;
    cur = it.cur;
    moved = false;
  }
  
  FaceVertIter& operator++()
  {  
    if (cur == NULL)
      return *this;
    
    cur = cur->next;
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  Vertex* operator*() const {
    return (Vertex*)(cur == NULL ? NULL : cur->vert);
  }

  Vertex* operator->() const {
    return (Vertex*)cur->vert;
  }
};

/*================================================
 *
 * Walks all half-edges in a circular loop
 *
 *==============================================*/

class HedgeLoopIter
{
  HMesh::HalfEdge *start;
  HMesh::HalfEdge *cur;
  bool moved;
  
public:
  HedgeLoopIter(HalfEdge *hedge)
  {
    if (hedge == NULL) {
      start = NULL;
      cur = NULL;
    }else{
      start = hedge;
      cur = hedge;
    }
    moved = false;
  }
  
  HedgeLoopIter(const HedgeLoopIter &it) {
    start = it.start;
    cur = it.cur;
    moved = it.moved;
  }
  
  HedgeLoopIter& operator++()
  {  
    if (cur == NULL)
      return *this;
    
    cur = cur->next;
    moved = true;
    return *this;
  }
  
  bool end() const {
    return
      moved && cur == start ||
      cur == NULL;
  }
  
  HalfEdge* operator*() const {
    return (HalfEdge*)cur;
  }

  HalfEdge* operator->() const {
    return (HalfEdge*)cur;
  }
};
