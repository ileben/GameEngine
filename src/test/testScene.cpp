#include "engine/geEngine.h"
#include "GL/glut.h"
using namespace GE;

int resX = 100;
int resY = 100;

void initGlut (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Game Editor" );
  /*
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyDown );
  glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  glutMouseFunc( click );
  glutMotionFunc( drag );
  glutIdleFunc( animate );
  idleDraw = true;
  */
}

int main (int argc, char **argv)
{
  initGlut( argc, argv );

  Kernel *kernel = new Kernel;

  CubeMesh *mesh = new CubeMesh;
  kernel->cacheResource( mesh, "testMesh" );

  Character *character = new Character;
  character->pose = new SkinPose;
  character->meshes[ 0 ] = new SkinTriMesh;
  kernel->cacheResource( character, "testChar" );

  Actor3D *root = new Actor3D;

  StandardMaterial *m1 = new StandardMaterial;
  DiffuseTexMat *m2 = new DiffuseTexMat;
  NormalTexMat *m3 = new NormalTexMat;
  MultiMaterial *mm = new MultiMaterial;
  mm->setNumSubMaterials( 3 );
  mm->setSubMaterial( 0, m1 );
  mm->setSubMaterial( 1, m2 );
  mm->setSubMaterial( 2, m3 );

  TriMeshActor *actor1 = new TriMeshActor;
  actor1->setMesh( mesh );
  actor1->setMaterial( mm );
  actor1->setParent( root );

  TriMeshActor *actor2 = new TriMeshActor;
  actor2->setMesh( mesh );
  actor2->setParent( root );

  SkinMeshActor *actor3 = new SkinMeshActor;
  actor3->setCharacter( character );
  actor3->setParent( root );

  void *data; UintSize size;
  
  SerializeManager sm;
  //sm.save( root, &data, &size );
  sm.save( character, &data, &size );
  sm.load( data );

  kernel->loadSceneData( data );

  return EXIT_SUCCESS;
}
