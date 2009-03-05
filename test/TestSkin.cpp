#include <engine/geEngine.h>
#include <engine/geGLHeaders.h>
using namespace GE;

#include <cstdlib>
#include <cstdio>

void applyFK( UintSize frame );

class SPolyActor : public PolyMeshActor { public:
  virtual void renderMesh (MaterialID materialID);
};


/*
==========================================================
State
==========================================================*/

enum CameraMode
{
  CAMERA_MODE_PAN,
  CAMERA_MODE_ORBIT,
  CAMERA_MODE_ZOOM
};

ByteString data;
MaxCharacter *character;

SPolyMesh *polyMesh;
SPolyActor *polyActor;
ArrayList <Vector3> polyPosePoints;
ArrayList <Vector3> polyPoseNormals;

SkinTriMesh *triMesh;
TriMeshActor *triActor;
ArrayList <Vector3> triPosePoints;
ArrayList <Vector3> triPoseNormals;

Scene *scene;
Light *light;

FpsLabel lblFps;

Camera2D cam2D;
Camera3D cam3D;
Renderer *renderer;
bool down3D;
Vector2 lastMouse3D;
int boneColorIndex = 0;
UintSize frame = 0;
UintSize numFrames = 0;
float curTime = 0.0f;
float maxTime = 0.0f;

int resY = 512;
int resX = 512;
Vector3 center( 0,0,0 );
Vector3 boundsMin( 0,0,0 );
Vector3 boundsMax( 0,0,0 );
CameraMode cameraMode;

//Makes toggling idle draw easier
bool idleDraw = false;
void INLINE postRedisplay()
{
  if (!idleDraw)
    glutPostRedisplay();
}

void drag3D (int x, int y)
{
  //printf ("Eye1:"); printVector (cam3D.getEye());
  //printf ("Look:"); printVector (cam3D.getLook());
  //printf ("Side:"); printVector (cam3D.getSide());
  //printf ("\n");
  
  //printf ("------------------------\n");
  //printMatrix (cam3D.getMatrix());
  
  Vector2 diff = Vector2( (Float)x,(Float)y ) - lastMouse3D;
  float eyeDist = ( cam3D.getEye() - center ).norm();
  
  Float angleH = diff.x * (2*PI) / 400;
  Float angleV = diff.y * (2*PI) / 400;
  Float panH = -diff.x * ( eyeDist * 0.002f );
  Float panV =  diff.y * ( eyeDist * 0.002f );
  Float zoom = -diff.y * ( eyeDist * 0.01f );
  lastMouse3D.set( (Float)x, (Float)y );
  
  switch (cameraMode)
  {  
  case CAMERA_MODE_ZOOM:

    cam3D.zoom( zoom );
    break;
    
  case CAMERA_MODE_ORBIT:
    
    cam3D.setCenter( center );
    cam3D.orbitH( angleH, true );
    cam3D.orbitV( angleV, true );
    break;

  case CAMERA_MODE_PAN:

    //cam3D.panH( panH );
    //cam3D.panV( panV );
    Vector3 flatLook = cam3D.getLook();
    flatLook.y = 0.0f; flatLook.normalize();
    Vector3 t = flatLook * -panV + cam3D.getSide() * -panH;
    light->translate( t.x, t.y, t.z );
    light->lookAt( center - light->getMatrix().getColumn(3).xyz(), Vector3(0,1,0) );
    break;
  }
  
  postRedisplay();
}

void click3D (int button, int state, int x, int y)
{
  if (state != GLUT_DOWN)
  {
    down3D = false;
    return;
  }

  int mods = glutGetModifiers();
  
  if (mods & GLUT_ACTIVE_SHIFT)
  {
    cameraMode = CAMERA_MODE_ORBIT;
  }
  else if ((mods & GLUT_ACTIVE_CTRL) ||
           button == GLUT_RIGHT_BUTTON)
  {
    cameraMode = CAMERA_MODE_ZOOM;
  }
  else
  {
    cameraMode = CAMERA_MODE_PAN;
  }
  
  lastMouse3D.set( (Float)x, (Float)y );
  down3D = true;
}

void click (int button, int state, int x, int y)
{
  click3D( button, state, x, y );
}

void drag (int x, int y)
{
  if (down3D)
    drag3D( x, y );
}

void keyboard (unsigned char key, int x, int y)
{
  switch (key)
  {
  case '+':
    //boneColorIndex++;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    if (frame < numFrames-1) ++frame;
    if (curTime < maxTime) curTime += 0.01f;
    applyFK( frame );
    break;
  case '-':
    //if (boneColorIndex > 0) boneColorIndex--;
    //printf ("BoneIndex: %d\n", boneColorIndex);
    if (frame > 0) --frame; 
    if (curTime > 0.0f) curTime -= 0.01f;
    applyFK( frame );
    break;
  case 27:
    //Quit on escape
    exit(0);
  }
}

void display ()
{
  renderer->renderShadowMap( light, scene );
  renderer->beginFrame();
  
  //switch camera
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( &cam3D );
  
  //draw model
  renderer->beginScene( scene );
  renderer->renderScene();
  renderer->endScene();
  
  //Frames per second
  renderer->setViewport( 0,0,resX, resY );
  renderer->setCamera( &cam2D );
  renderer->renderWidget( &lblFps );
  
  renderer->endFrame();
}

void reshape (int w, int h)
{
  resX = w;
  resY = h;
}

void findCenter ()
{
  int count = 0;
  center.set( 0,0,0 );
  boundsMin.set( 0,0,0 );
  boundsMax.set( 0,0,0 );

  PolyMesh *mesh = polyActor->getMesh();
  for (PolyMesh::VertIter v(mesh); !v.end(); ++v) {
    center += v->point;
    count++;
  }
  
  center /= (Float)count;
}

void cleanup()
{
}

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Test Skin" );
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutMouseFunc( click );
  glutMotionFunc( drag );
  glutIdleFunc( display );
  idleDraw = true;
}

class VertColorMaterial : public StandardMaterial { public:
  virtual void begin ()  {
    StandardMaterial::begin ();
    glEnable( GL_COLOR_MATERIAL );
  }
};

void SPolyActor::renderMesh (MaterialID matid)
{
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  TexMesh::FaceIter uf( texMesh );
  for (SPolyMesh::FaceIter f( polyMesh ); !f.end(); ++f, ++uf) {
    
    //Check if this face belongs to current material
    if (matid != f->materialID() &&
        matid != GE_ANY_MATERIAL_ID)
      continue;
    
    glBegin( GL_POLYGON );
    
    TexMesh::FaceVertIter uv(*uf);
    for(SPolyMesh::FaceHedgeIter h(*f); !h.end(); ++h, ++uv) {
      
      //Adjust the color based on bone weight
      SPolyMesh::Vertex *vert = h->dstVertex();
      glColor3f (1,1,1);
      for (int b=0; b<4; ++b) {
        if (vert->boneIndex[ b ] == boneColorIndex &&
            vert->boneWeight[ b ] > 0.0f)
            glColor3f( 1, 0, 0 ); }
      
      //Vertex normal
      glNormal3fv( (Float*) &h->vertexNormal()->coord );
      
      //UV coordinates
      if (!uv.end()) {
        glTexCoord2f( uv->point.x, uv->point.y ); }
      
      //Vertex coordinate
      glVertex3fv( (Float*) &h->dstVertex()->point );
    }
    
    glEnd();
  }
}

void loadPackage (String fileName)
{ 
  //Read the file
  File file( fileName );
  if( !file.open( "rb" ))
  {
    printf( "Failed opening file!\n" );
    getchar();
    exit( 1 );
  }
  
  data = file.read( file.getSize() );
  file.close();
  
  //Load character data
  SerializeManager sm;
  character = (MaxCharacter*) sm.load( (void*)data.buffer() );
  
  SkinMesh *inMesh = character->mesh;
  numFrames = character->anims.first()->tracks.first()->keys.size();
  maxTime = character->anims.first()->tracks.first()->totalTime;

  SkinAnim *anim = character->anims.first();
  
  printf ("Imported %d verts, %d faces, %d indices, %d animations\n",
          inMesh->verts.size(),
          inMesh->faces.size(),
          inMesh->indices.size(),
          character->anims.size());

  printf ("Animation name: '%s'\n",
          character->anims.first()->name.buffer());

  
  //Add vertices to the mesh
  polyMesh = new SPolyMesh;
  ArrayList <SPolyMesh::Vertex*> verts( inMesh->verts.size() );
  for (UintSize v=0; v<inMesh->verts.size(); ++v)
  {
    SPolyMesh::Vertex *vert = (SPolyMesh::Vertex*) polyMesh->addVertex();
    vert->point = inMesh->verts[v].point;
    for (int b=0; b<4; ++b) {
      vert->boneIndex[b] = inMesh->verts[v].boneIndex[b];
      vert->boneWeight[b] = inMesh->verts[v].boneWeight[b]; }
    verts.pushBack (vert);
  }
  
  //Add indexed faces to the mesh
  int nextIndex = 0;
  for (UintSize f=0; f<inMesh->faces.size(); ++f)
  {
    int numCorners = inMesh->faces[f].numCorners;
    HMesh::Vertex **corners = new HMesh::Vertex*[ numCorners ];
    
    for (int c=0; c<numCorners; ++c) {
      Uint32 vertIndex = inMesh->indices[ nextIndex++ ];
      if (vertIndex > inMesh->verts.size()) {
        printf( "Invalid vertex: %d\n", vertIndex );
        vertIndex = 0; }
      corners[c] = verts[ vertIndex ];
    }
    
    SPolyMesh::Face *face = (SPolyMesh::Face*) polyMesh->addFace( corners, numCorners );
    if (face != NULL) face->smoothGroups = inMesh->faces[f].smoothGroups;
    
    delete[] corners;
  }
  
  polyMesh->updateNormals( SHADING_SMOOTH );
  
  //Convert to TriMesh
  triMesh = new SkinTriMesh;
  triMesh->fromPoly( polyMesh, NULL );
  
  //Store original normals and points
  for (SPolyMesh::VertIter vi( polyMesh ); !vi.end(); ++vi)
    polyPosePoints.pushBack( vi->point );
  
  for (SPolyMesh::VertexNormalIter ni( polyMesh ); !ni.end(); ++ni)
    polyPoseNormals.pushBack( ni->coord );
  
  ArrayList <SkinTriMesh::Vertex> *triVerts =
    (ArrayList <SkinTriMesh::Vertex> *) &triMesh->data;
  
  for (UintSize v=0; v<triVerts->size(); ++v)
  {
    triPosePoints.pushBack( triVerts->at( v ).point );
    triPoseNormals.pushBack( triVerts->at( v ).normal );
  }
}

void applyFK (UintSize frame)
{
  SkinPose *pose = character->pose;
  SkinAnim *anim = character->anims.first();
  ArrayList <Matrix4x4> fkMats;
  ArrayList <Matrix4x4> skinMats;
  int cindex = 1;
  
  UintSize numTracks = anim->tracks.size();
  UintSize numKeys = anim->tracks.first()->keys.size();
  
  //Root FK matrix = local matrix
  Matrix4x4 rootWorld;
  //rootWorld.fromQuat( anim->tracks->first()->keys->at( frame ).value);
  rootWorld.fromQuat( anim->tracks.first()->evalAt( curTime ));
  rootWorld.setColumn( 3, pose->bones.first().localT );
  rootWorld *= pose->bones.first().localS;
  fkMats.pushBack( rootWorld );
  
  //Walk all the bones
  for (UintSize b=0; b<pose->bones.size(); ++b)
  {
    //Final skin matrix = FK matrix * world matrix inverse
    SkinBone *parent = &pose->bones[b];
    skinMats.pushBack( fkMats[b] * parent->worldInv );
    
    //Walk the children
    for (Uint32 c=0; c<parent->numChildren; ++c)
    {
      //Child FK matrix = parent FK matrix * local matrix
      SkinBone *child = &pose->bones[ cindex ];
      SkinTrack *track = anim->tracks[ cindex ];
      cindex++;
      
      Matrix4x4 childLocal;
      //childLocal.fromQuat( track->keys->at( frame ).value);
      childLocal.fromQuat( track->evalAt( curTime ));
      childLocal.setColumn( 3, child->localT );
      childLocal *= child->localS;
      fkMats.pushBack( fkMats[b] * childLocal );
    }
  }

  int vindex = 0; int nindex = 0;
  /*
  //Transform vertices
  for (SPolyMesh::VertIter v(polyMesh); !v.end(); ++v, ++vindex)
  {
    v->point.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      Vector3 &posePoint = polyPosePoints[ vindex ];
      Vector3 skinPoint = skinMats[ v->boneIndex[i] ] * posePoint;
      v->point += skinPoint * v->boneWeight[i];
    }
  }
  
  //Transform normals
  for (SPolyMesh::VertexNormalIter n(polyMesh); !n.end(); ++n, ++nindex)
  {
    n->coord.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      SPolyMesh::Vertex *v = (SPolyMesh::Vertex*) n->vert;
      Vector3 &poseNormal = polyPoseNormals[ nindex ];
      Vector3 skinNormal = skinMats[ v->boneIndex[i] ].transformVector( poseNormal );
      n->coord += skinNormal * v->boneWeight[i];
    }
  } */
  
  ArrayList <SkinTriMesh::Vertex> *triVerts =
    (ArrayList <SkinTriMesh::Vertex> *) &triMesh->data;
  
  for (UintSize index=0; index<triVerts->size(); ++index)
  {
    SkinTriMesh::Vertex &v = triVerts->at( index );
    
    v.point.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      Vector3 &posePoint = triPosePoints[ index ];
      Vector3 skinPoint = skinMats[ v.boneIndex[i] ] * posePoint;
      v.point += skinPoint * v.boneWeight[i];
    }
    
    v.normal.set( 0,0,0 );
    for (int i=0; i<4; ++i)
    {
      Vector3 &poseNormal = triPoseNormals[ index ];
      Vector3 skinNormal = skinMats[ v.boneIndex[i] ].transformVector( poseNormal );
      v.normal += skinNormal * v.boneWeight[i];
    }
  }
}

class DD
{
  DECLARE_SERIAL_CLASS( DD );
  DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
  DECLARE_END;
public:
  int d;
  DD() {}
  DD( SM *sm ) {}
  void serialize( void *sm )
  {
    int poke = 1;
  }
};

DEFINE_SERIAL_CLASS( DD, ClassID(2,2,2,2) );

class CC
{
  DECLARE_SERIAL_CLASS( CC );
  DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
  DECLARE_END;
public:
  ClassArrayList<DD> list;
  CC () {};//{list = new ClassArrayList<DD>;}
  CC (SM *sm) : list (sm) {}
  void serialize (void *sm)
  {
    ((SM*)sm)->objectVar( &list );
  }
};

DEFINE_SERIAL_CLASS( CC, ClassID(1,1,1,1) );

/*
class DD
{
public:
  DD() { printf( "DD::Ctor()\n" ); }
  DD( int a ) { printf( "DD::Ctor(%d)\n", a ); }
};

class DD2 : public DD
{
public:
  DD2() { printf( "DD2:Ctor()\n" ); }
  DD2( int a ) : DD(a) { printf( "DD2::Ctor(%d)\n", a ); }
};

class CC
{
public:
  DD2 dd;
  CC() { printf( "CC::Ctor\n" ); }
  ~CC() { printf( "~CC::Dtor\n" ); }
  CC( int a ) : dd(a) { printf( "CC::Ctor(%d)\n", a ); }
};*/

int main (int argc, char **argv)
{
  /*
  ArrayList<CC> *list = new ArrayList<CC>;
  CC cc;
  
  list->pushBack( cc );
  list->pushBack( cc );
  list->pushBack( cc );
  list->pushBack( cc );

  delete list;
  
  getchar();
  return 0;
  */

  /*
  CC cc;
  for( int d=0; d<5; ++d ){
    cc.list.pushBack( new DD );
    cc.list.last()->d = d;
  }

  SM sm;
  void *data;
  UintSize size;
  sm.save( &cc, &data, &size );

  CC *ccc = (CC*) sm.load( data );
  return 0;*/
  /*
  SkinVertex vert1, vert2, vert3;
  vert1.point.set( 1,2,3 );
  vert2.point.set( 4,5,6 );
  vert3.point.set( 7,8,9 );

  SkinMesh *mesh = new SkinMesh;
  mesh->verts->pushBack( vert1 );
  mesh->verts->pushBack( vert2 );
  mesh->verts->pushBack( vert3 );

  SkinPose *pose = new SkinPose;
  SkinAnim *anim = new SkinAnim;

  MaxCharacter mchar;
  mchar.mesh = mesh;
  mchar.pose = pose;
  mchar.anim = anim;

  SM sm;
  void *data;
  UintSize size;
  //sm.serialize (&mchar, &data, &size);
  //sm.deserialize (data);
  sm.save (&mchar, &data, &size);
  MaxCharacter *outChar = (MaxCharacter*) sm.load (data);
  */

  //Initialize GLUT
  initGlut( argc, argv );
  
  Kernel kernel;
  kernel.enableVerticalSync( false );
  renderer = kernel.getRenderer();
  printf( "Kernel loaded\n" );
  
  //Setup camera
  cam3D.setCenter( center );
  cam3D.translate( 0,0,-200 );
  cam3D.orbitV( Util::DegToRad( 20 ), true );
  cam3D.orbitH( Util::DegToRad( 30 ), true );
  cam3D.setNearClipPlane( 10.0f );
  cam3D.setFarClipPlane( 1000.0f );
  
  Shader *shader = new Shader;
  shader->fromFile( "pixelphong.vert.c", "pixelphong.frag.c" );
  //shader->registerUniform( "sampler", GE_UNIFORM_TEXTURE, 1 );

  //VertColorMaterial mat;
  StandardMaterial mat;
  //PhongMaterial mat;
  mat.setSpecularity( 0.5 );
  //mat.setCullBack( false );
  mat.setShader( shader );

  scene = new Scene;
  
  loadPackage( "bub.pak" );
  
  polyActor = new SPolyActor;
  polyActor->setMaterial( &mat );
  polyActor->setMesh( polyMesh );
  
  triActor = new TriMeshActor;
  triActor->setMaterial( &mat );
  triActor->setMesh( triMesh );
  triActor->rotate( Vector3(0,1,0), Util::DegToRad(180) );
  scene->addChild( triActor );
  
  applyFK( 0 );

  TriMesh *cubeMesh = new CubeMesh;
  TriMeshActor *cube = new TriMeshActor;
  cube->setMaterial( &mat );
  cube->setMesh( cubeMesh );
  cube->scale( 300, 10, 300 );
  cube->translate( 0, -70, 0 );
  scene->addChild( cube );

  StandardMaterial axesMat;
  axesMat.setUseLighting( false );
  AxisActor *axes = new AxisActor;
  axes->scale( 100 );
  axes->setMaterial( &axesMat );
  //scene->addChild( axes );

  light = new SpotLight( Vector3(-200,200,-200), Vector3(1,-1,1), 60, 0 );
  scene->addChild( light );
  
  lblFps.setLocation( Vector2( 0.0f, (Float)resY ));
  lblFps.setColor( Vector3( 1.0f, 1.0f, 1.0f ));
  
  //Find model center
  findCenter();
  cam3D.setCenter( center );

  //Run application
  atexit( cleanup );
  glutMainLoop();
  cleanup();

  return EXIT_SUCCESS;
}
