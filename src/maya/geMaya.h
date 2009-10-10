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
  DECLARE_SERIAL_SUBCLASS( MayaEventDummy, Object );
  DECLARE_OBJVAR( name );
  DECLARE_DATAVAR( time );
  DECLARE_END;

public:

  CharString name;
  Float time;

  MayaEventDummy (SM *sm) : Object (sm), name (sm) {}
  MayaEventDummy () : time (0.0f) {}

  bool operator < (const MayaEventDummy &other) {
    return time < other.time;
  }
};

class MayaClipDummy : public Object
{
  DECLARE_SERIAL_SUBCLASS( MayaClipDummy, Object );
  DECLARE_DATAVAR( type );
  DECLARE_OBJVAR( nodePath );
  DECLARE_DATAVAR( startTime );
  DECLARE_DATAVAR( endTime );
  DECLARE_END;

public:

  MayaClipType::Enum type;
  CharString nodePath;
  Float startTime;
  Float endTime;

  MayaClipDummy () {}
  MayaClipDummy (SM *sm) : Object (sm), nodePath (sm) {}

  bool operator < (const MayaClipDummy &other) {
    return startTime < other.startTime;
  }
};

class MayaAnimDummy : public Object
{
  DECLARE_SERIAL_SUBCLASS( MayaAnimDummy, Object );
  DECLARE_OBJVAR( name );
  DECLARE_DATAVAR( startTime );
  DECLARE_DATAVAR( endTime );
  DECLARE_OBJVAR( clips );
  DECLARE_OBJVAR( events );
  DECLARE_END;

public:

  CharString name;
  Float startTime;
  Float endTime;
  ObjPtrArrayList< MayaClipDummy > clips;
  ObjPtrArrayList< MayaEventDummy > events;

  MayaAnimDummy () {}
  MayaAnimDummy (SM *sm) : Object (sm), name (sm), clips (sm), events (sm) {}
  virtual ~MayaAnimDummy ()
  {
    for (UintSize c=0; c<clips.size(); ++c)
      delete clips[ c ];
    for (UintSize e=0; e<events.size(); ++e)
      delete events[ e ];
  }

  bool operator < (const MayaAnimDummy &other) {
    return startTime < other.startTime;
  }
};

class MayaSceneDummy : public Object
{
  DECLARE_SERIAL_SUBCLASS( MayaSceneDummy, Object );
  DECLARE_OBJVAR( anims );
  DECLARE_END;

public:

  ObjPtrArrayList< MayaAnimDummy > anims;

  MayaSceneDummy () {}
  MayaSceneDummy (SM *sm) : Object (sm), anims (sm) {}
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
