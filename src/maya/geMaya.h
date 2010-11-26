#ifndef __GEMAYA_H
#define __GEMAYA_H

#include <iostream> //Maya needs this
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MAnimControl.h>
#include <maya/MFnCamera.h>

#include <util/geUtil.h>
#include <engine/geEngine.h>
using namespace GE;

namespace MayaClipType {
  enum Enum
  {
    ActorTransform   = 1,
    SkinTransform    = 2,
    DepthOfField     = 3
  };
}

class MayaEventDummy : public Object
{
  CLASS( MayaEventDummy, Object,
    567d5635,60b4,4fdd,84c0987700408c35 );

  virtual void serialize( Serializer *s, Uint v )
  {
    Object::serialize( s,v );
    s->string( &name );
    s->data( &time );
  }

public:

  CharString name;
  Float time;
  MayaEventDummy () : time (0.0f) {}

  bool operator < (const MayaEventDummy &other) {
    return time < other.time;
  }
};

class MayaClipDummy : public Object
{
  CLASS( MayaClipDummy, Object,
    6dd377d3,23e7,4066,8bb01fdc059f138c );

  virtual void serialize( Serializer *s, Uint v )
  {
    Object::serialize( s,v );
    s->data( &type );
    s->string( &nodePath );
    s->data( &startTime );
    s->data( &endTime );
  }

public:

  MayaClipType::Enum type;
  CharString nodePath;
  Float startTime;
  Float endTime;

  bool operator < (const MayaClipDummy &other) {
    return startTime < other.startTime;
  }
};

class MayaAnimDummy : public Object
{
  CLASS( MayaAnimDummy, Object,
    24191b03,bc6e,4320,b5d59502ebce394b );

  virtual void serialize( Serializer *s, Uint v )
  {
    Object::serialize( s,v );
    s->string( &name );
    s->data( &startTime );
    s->data( &endTime );
    s->objectPtrArray( &clips );
    s->objectPtrArray( &events );
  }

public:

  CharString name;
  Float startTime;
  Float endTime;
  ArrayList< MayaClipDummy* > clips;
  ArrayList< MayaEventDummy* > events;

  bool operator < (const MayaAnimDummy &other) {
    return startTime < other.startTime;
  }

  virtual ~MayaAnimDummy ()
  {
    for (UintSize c=0; c<clips.size(); ++c)
      delete clips[ c ];
    for (UintSize e=0; e<events.size(); ++e)
      delete events[ e ];
  }
};

class MayaSceneDummy : public Object
{
  CLASS( MayaSceneDummy, Object,
    88693913,d9a5,4229,ae06cb3659bf492f );

  virtual void serialize( Serializer *s, Uint v )
  {
    Object::serialize( s,v );
    s->objectPtrArray( &anims );
  }

public:

  ArrayList< MayaAnimDummy* > anims;

  virtual ~MayaSceneDummy ()
  {
    for (UintSize a=0; a<anims.size(); ++a)
      delete anims[ a ];
  }
};

File getProjectFolder ();
Float getWorldScale ();
void trace( const CharString &s);
void setStatus (const CharString &msg);
void clearStatus ();

Actor3D* findActorByName (const CharString &name);
bool findNodeByType (MFn::Type type, MDagPath &outPath, MDagPath *start = NULL);
bool findNodeByName (const CharString &name, MDagPath &outPath);
void findNodesInSelection (MFn::Type type, ArrayList< MDagPath > &outPaths);
bool findNodeInSelection (MFn::Type type, MObject &pick);
bool findSkinForMesh (const MObject &meshNode, MObject &skinNode);
bool findSkinJointRoot (const MObject &skinNode, MObject &rootJoint);

Matrix4x4 exportMatrix (const MMatrix &m);
Vector3 exportPoint (const MFloatPoint &p);
Vector3 exportVector (const MVector &v);
Vector2 exportUV (double u, double v);
Quat exportQuat (const MQuaternion &q);
Matrix4x4 exportScale (double s[3]);
Vector3 exportColor (const MColor &c);

TriMesh* exportMesh (const MObject &meshNode, bool tangents);
Character* exportCharacter (const MObject &meshNode, bool tangents);
Material* exportMaterial (const MDagPath &nodePath, bool *hasNormalMap);
Matrix4x4 exportMatrix (const MMatrix &m);
Light* exportLight (const MDagPath &lightDagPath);
Camera* exportCamera (const MDagPath &camDagPath);
Animation* exportSkinAnimation (int start, int end, int fps);
Animation* exportAnimation (int kps, MayaAnimDummy *anim);

#endif//__GEMAYA_H
