#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (LoaderObj);
  
  void LoaderObj::readLine()
  {
    char c = 0;
    linePointer = 0;
    line.reserve(20);

    do {
      
      //Try to read a character
      if (bufPointer == buffer.length()) {
        endOfFile=true; return;}
      c = buffer[bufPointer++];
      
      //Resize string capacity
      if (line.length() == line.capacity())
        line.reserveAndCopy(2 * line.capacity());
      
      //Add a valid character
      if (c != '\n' && c != '\r')
        line += c;

      //Until new line met
    } while (c != '\n');
  }

  bool LoaderObj::isWhitespace(char c) {
    return (c == ' '  || c == '\t');
  }

  ByteString LoaderObj::parseToken()
  {
    int start = 0;
    int end = 0;

    //Jump over any whitespace
    for (start = linePointer; start < line.length(); ++start)
      if (!isWhitespace(line[start])) break;
    
    //Walk string until whitespace found
    for (end = start; end < line.length(); ++end)
      if (isWhitespace(line[end])) break;

    //Position line pointer and return token
    linePointer = end;
    return line.sub(start, end - start);
  }

  Vector3 LoaderObj::parseVector3()
  {
    //Parse three float coords
    ByteString strx, stry, strz;
    strx = parseToken();
    stry = parseToken();
    strz = parseToken();
    
    Vector3 v;
    v.x = strx.parseFloat();
    v.y = stry.parseFloat();
    v.z = strz.parseFloat();
    
    return v;
  }

  void LoaderObj::command_Vertex()
  {
    points.pushBack(parseVector3());
  }

  void LoaderObj::command_UVW()
  {
    Vector3 uvw = parseVector3();
    ucoords.pushBack(Vector3(uvw.x, 1.0f - uvw.y, uvw.z));
  }

  void LoaderObj::command_Normal()
  {
    ncoords.pushBack(parseVector3());
  }

  void LoaderObj::command_SmoothGroup()
  {
    smoothGroup = parseToken().parseInteger();
  }

  void LoaderObj::newShape(const ByteString &name)
  {
    //Create new shape with given id
    shape = new PolyMeshActor;
    umesh = (UMesh*) New(uvMeshClass);
    mesh = (PolyMesh*) New(dMeshClass);
    
    shape->setId (name);
    shape->setMesh (mesh);
    shape->setTexMesh (umesh);

    resources.pushBack(umesh);
    resources.pushBack(mesh);
    objects.pushBack(shape);

    //Initialize space for the vertices of the new mesh.
    //They are created only if really used by this mesh
    //once referenced for the first time

    verts.reserve(points.size());
    for (int i=0; i<points.size(); ++i)
      verts.pushBack(NULL);

    uverts.reserve(ucoords.size());
    for (int u=0; u<ucoords.size(); ++u)
      uverts.pushBack(NULL);

    snormals.reserve(ncoords.size());
    for (int n=0; n<ncoords.size(); ++n)
      snormals.pushBack(NULL);
  }

  void LoaderObj::command_Group()
  {
    //End previous shape and start new
    newShape(parseToken());
  }

  void LoaderObj::command_Face()
  {
    //New shape if none present
    if (shape == NULL) newShape("");

    //Add space for new vertices so far. They are created
    //only if really used by this mesh once referenced
    //for the first time

    verts.reserveAndCopy(points.size());
    for (int p=verts.size(); p<points.size(); ++p)
      verts.pushBack(NULL);

    uverts.reserveAndCopy(ucoords.size());
    for (int u=uverts.size(); u<ucoords.size(); ++u)
      uverts.pushBack(NULL);

    snormals.reserveAndCopy(ncoords.size());
    for (int n=snormals.size(); n<ncoords.size(); ++n)
      snormals.pushBack(NULL);

    //Temp arrays for face indices
    VertArray faceVerts(4);
    UVertArray faceUverts(4);
    SNormalArray faceSnormals(4);

    //Parse face vertices
    ArrayList<ByteString> ints(3);
    ByteString sf = parseToken();
    while (sf != "") {
      
      //Get indices
      ints.clear();
      sf.tokenize("/", &ints);

      if (ints.empty()) break;
      int ivert=-1, iuvert=-1, inormal=-1;
      
      //Parse vertex index
      ivert = ints[0].parseInteger();
      if (ivert >= 0) ivert -= 1; //zero based!
      else ivert = verts.size() + ivert; //negative means backwards

      //Parse UV vertex index
      if (ints.size() >= 2) {
        iuvert = ints[1].parseInteger();
        if (iuvert >= 0) iuvert -= 1;
        else iuvert = uverts.size() + iuvert; }

      //Parse normal index
      if (ints.size() >= 3) {
        inormal = ints[2].parseInteger();
        if (inormal >= 0) inormal -= 1;
        else inormal = snormals.size() + inormal; }

      //Skip invalid vertex
      bool vertOk = (ivert >= 0 && ivert < verts.size());
      bool uvertOk  = (iuvert >= 0 && iuvert < uverts.size());
      bool normalOk = (inormal >= 0 && inormal < snormals.size());

      if (vertOk) {

        //Create new vertex if first time referenced
        if (verts[ivert] == NULL) {
          verts[ivert] = (PolyMesh::Vertex*)mesh->addVertex();
          verts[ivert]->point = points[ivert]; }
        faceVerts.pushBack(verts[ivert]);

        //Create new UV vertex if first time referenced
        if (uvertOk) {
          if (uverts[iuvert] == NULL) {
            uverts[iuvert] = (UMesh::Vertex*)umesh->addVertex();
            uverts[iuvert]->point = ucoords[iuvert].xy(); }
          faceUverts.pushBack(uverts[iuvert]); }

        //Create new SmoothNormal if first time referenced
        if (normalOk) {
          if (snormals[inormal] == NULL) {
            mesh->smoothNormals.pushBack (PolyMesh::SmoothNormal (ncoords[inormal]));
            snormals[inormal] = &mesh->smoothNormals.last(); }
          faceSnormals.pushBack(snormals[inormal]); }
      }

      //Pick next vertex data
      sf = parseToken();
    }
    
    //Create mesh face if at least triangle
    if (faceVerts.size() < 3) return;
    PolyMesh::Face *face = (PolyMesh::Face*)mesh->addFace(
      (HMesh::Vertex**)faceVerts.buffer(), faceVerts.size());
    if (face == NULL) return;
    face->smoothGroups = smoothGroup;

    //Create UV mesh face if at least triangle
    if (faceUverts.size() >= 3)
      umesh->addFace((HMesh::Vertex**)faceUverts.buffer(), faceUverts.size());

    //Apply normals to mesh face
    PolyMesh::HalfEdge *h = face->hedgeTo(faceVerts[0]);
    int i=0; for (PolyMesh::HedgeLoopIter l(h); !l.end(); ++l, ++i) {
       
      //Check if normal coord present and store
      if (i < faceSnormals.size())
        l->snormal = faceSnormals[i];
    }
  }


  bool LoaderObj::loadFile(const OCC::String &filename)
  {
    //Try to open file
    FileRef module = File::GetModule();
    file = module->getRelativeFile(filename);
    if (!file->open("rb")) return false;

    //Read whole file into a buffer
    bufPointer = 0;
    buffer.clear();
    file->read(buffer, file->getSize());

    //Reset loader
    endOfFile = false;
    linePointer = 0;
    line = "";
    
    points.clear();
    ncoords.clear();
    ucoords.clear();
    
    shape = NULL;
    umesh = NULL;
    mesh = NULL;
    smoothGroup = 0;
    uverts.clear();
    verts.clear();
    snormals.clear();
    int counter = 0;


    //Walk lines
    do {

      readLine();
      if (endOfFile) break;

      
      ByteString command = parseToken();

      if (command == "v") {
        command_Vertex();

      }else if (command == "vn") {
        command_Normal();

      }else if (command == "vt") {
        command_UVW();

      }else if (command == "f") {
        command_Face();

      }else if (command == "g") {
        command_Group();

      }else if (command == "s") {
        command_SmoothGroup();
      }

    }while (!endOfFile);


    file->close();
    return true;
  }

}/* namespace GE */
